// Mini memory editor for Dear ImGui (to embed in your game/tools)
// Get latest version at http://www.github.com/ocornut/imgui_club
//
// Right-click anywhere to access the Options menu!
// You can adjust the keyboard repeat delay/rate in ImGuiIO.
// The code assume a mono-space font for simplicity!
// If you don't use the default font, use igPushFont()/PopFont() to switch to a
// mono-space font before caling this.
//
// Usage:
//   // Create a window and draw memory editor inside it:
//   static MemoryEditor mem_edit_1;
//   static char data[0x10000];
//   size_t data_size = 0x10000;
//   mem_edit_1.DrawWindow("Memory Editor", data, data_size);
//
// Usage:
//   // If you already have a window, use DrawContents() instead:
//   static MemoryEditor mem_edit_2;
//   igBegin("MyWindow")
//   mem_edit_2.DrawContents(this, sizeof(*this), (size_t)this);
//   igEnd();
//
// Changelog:
// - v0.10: initial version
// - v0.23 (2017/08/17): added to github. fixed right-arrow triggering a byte
// write.
// - v0.24 (2018/06/02): changed DragInt("Rows" to use a %d data format (which
// is desirable since imgui 1.61).
// - v0.25 (2018/07/11): fixed wording: all occurrences of "Rows" renamed to
// "Columns".
// - v0.26 (2018/08/02): fixed clicking on hex region
// - v0.30 (2018/08/02): added data preview for common data types
// - v0.31 (2018/10/10): added OptUpperCaseHex option to select lower/upper
// casing display [@samhocevar]
// - v0.32 (2018/10/10): changed signatures to use void* instead of unsigned
// char*
// - v0.33 (2018/10/10): added OptShowOptions option to hide all the interactive
// option setting.
// - v0.34 (2019/05/07): binary preview now applies endianness setting
// [@nicolasnoble]
// - v0.35 (2020/01/29): using ImGuiDataType available since Dear ImGui 1.69.
// - v0.36 (2020/05/05): minor tweaks, minor refactor.
// - v0.40 (2020/10/04): fix misuse of ImGuiListClipper API, broke with Dear
// ImGui 1.79. made cursor position appears on left-side of edit box. option
// popup appears on mouse release. fix MSVC warnings where
// _CRT_SECURE_NO_WARNINGS wasn't working in recent versions.
// - v0.41 (2020/10/05): fix when using with keyboard/gamepad navigation
// enabled.
// - v0.42 (2020/10/14): fix for . character in ASCII view always being greyed
// out.
//
// Todo/Bugs:
// - This is generally old code, it should work but please don't use this as
// reference!
// - Arrows are being sent to the InputText() about to disappear which for
// LeftArrow makes the text cursor appear at position 1 for one frame.
// - Using InputText() is awkward and maybe overkill here, consider implementing
// something custom.

#pragma once

#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#define IM_COL32(R, G, B, A)                                                   \
  (((ImU32)(A) << IM_COL32_A_SHIFT) | ((ImU32)(B) << IM_COL32_B_SHIFT) |       \
   ((ImU32)(G) << IM_COL32_G_SHIFT) | ((ImU32)(R) << IM_COL32_R_SHIFT))
#define IM_COL32_WHITE IM_COL32(255, 255, 255, 255) // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK IM_COL32(0, 0, 0, 255)       // Opaque black
#define IM_COL32_BLACK_TRANS                                                   \
  IM_COL32(0, 0, 0, 0) // Transparent black = 0x00000000

#ifndef IM_ASSERT
#include <assert.h>
#define IM_ASSERT(_EXPR)                                                       \
  assert(_EXPR) // You can override the default assert handler by editing
                // imconfig.h
#endif
#if !defined(IMGUI_USE_STB_SPRINTF) && (defined(__clang__) || defined(__GNUC__))
#define IM_FMTARGS(FMT)                                                        \
  __attribute__(                                                               \
      (format(printf, FMT,                                                     \
              FMT + 1))) // To apply printf-style warnings to our functions.
#define IM_FMTLIST(FMT) __attribute__((format(printf, FMT, 0)))
#else
#define IM_FMTARGS(FMT)
#define IM_FMTLIST(FMT)
#endif
#define IM_ARRAYSIZE(_ARR)                                                     \
  ((int)(sizeof(_ARR) / sizeof(*(_ARR)))) // Size of a static C-style array.
                                          // Don't use on pointers!
#define IM_UNUSED(_VAR)                                                        \
  ((void)(_VAR)) // Used to silence "unused variable warnings". Often useful as
                 // asserts may be stripped out from final builds.
#if (__cplusplus >= 201100)
#define IM_OFFSETOF(_TYPE, _MEMBER)                                            \
  offsetof(_TYPE, _MEMBER) // Offset of _MEMBER within _TYPE. Standardized as
                           // offsetof() in C++11
#else
#define IM_OFFSETOF(_TYPE, _MEMBER)                                            \
  ((size_t) &                                                                  \
   (((_TYPE *)0)                                                               \
        ->_MEMBER)) // Offset of _MEMBER within _TYPE. Old style macro.
#endif

#include <float.h>
#include <memory.h>
#include <stdint.h> // uint8_t, etc.
#include <stdio.h>  // sprintf, scanf
#include <stdlib.h>

#ifdef _MSC_VER
#define _PRISizeT "I"
#define ImSnprintf _snprintf
#else
#define _PRISizeT "z"
#define ImSnprintf snprintf
#endif

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

typedef enum {
  DataFormat_Bin = 0,
  DataFormat_Dec = 1,
  DataFormat_Hex = 2,
  DataFormat_COUNT
} DataFormat;

typedef struct {
  // Settings
  bool Open; // = true   // set to false when DrawWindow() was closed. ignore if
             // not using DrawWindow().
  bool ReadOnly;       // = false  // disable any editing.
  int Cols;            // = 16     // number of columns to display.
  bool OptShowOptions; // = true   // display options button/context menu. when
                       // disabled, options will be locked unless you provide
                       // your own UI for them.
  bool OptShowDataPreview; // = false  // display a footer previewing the
                           // decimal/binary/hex/float representation of the
                           // currently selected bytes.
  bool OptShowHexII;       // = false  // display values in HexII representation
                     // instead of regular hexadecimal: hide null/zero bytes,
                     // ascii values as ".X".
  bool OptShowAscii; // = true   // display ASCII representation on the right
                     // side.
  bool OptGreyOutZeroes; // = true   // display null/zero bytes using the
                         // TextDisabled color.
  bool OptUpperCaseHex;  // = true   // display hexadecimal values as "FF"
                         // instead of "ff".
  int OptMidColsCount; // = 8      // set to 0 to disable extra spacing between
                       // every mid-cols.
  int OptAddrDigitsCount; // = 0      // number of addr digits to display
                          // (default calculated based on maximum displayed
                          // addr).
  ImU32 HighlightColor;   //          // background color of highlighted bytes.
  ImU8 (*ReadFn)(const ImU8 *data,
                 size_t off); // = 0      // optional handler to read bytes.
  void (*WriteFn)(ImU8 *data, size_t off,
                  ImU8 d); // = 0      // optional handler to write bytes.
  bool (*HighlightFn)(
      const ImU8 *data,
      size_t off); //= 0      // optional handler to return Highlight property
                   //(to support non-contiguous highlighting).

  // [Internal State]
  bool ContentsWidthChanged;
  size_t DataPreviewAddr;
  size_t DataEditingAddr;
  bool DataEditingTakeFocus;
  char DataInputBuf[32];
  char AddrInputBuf[32];
  size_t GotoAddr;
  size_t HighlightMin, HighlightMax;
  int PreviewEndianess;
  ImGuiDataType PreviewDataType;
} GBMemoryEditor;

typedef struct {
  char CurrentBufOverwrite[3]; // Input
  int CursorPos;               // Output
} GBMemoryEditorUserData;

// FIXME: We should have a way to retrieve the text edit cursor
// position more easily in the API, this is rather tedious. This is
// such a ugly mess we may be better off not using InputText() at all
// here.`
int GBMemoryEditorUserDataCallback(ImGuiInputTextCallbackData *data) {
  GBMemoryEditorUserData *user_data = (GBMemoryEditorUserData *)data->UserData;
  if (!ImGuiInputTextCallbackData_HasSelection(data))
    user_data->CursorPos = data->CursorPos;
  if (data->SelectionStart == 0 && data->SelectionEnd == data->BufTextLen) {
    // When not editing a byte, always rewrite its content (this is a
    // bit tricky, since InputText technically "owns" the master copy
    // of the buffer we edit it in there)
    ImGuiInputTextCallbackData_DeleteChars(data, 0, data->BufTextLen);
    ImGuiInputTextCallbackData_InsertChars(
        data, 0, user_data->CurrentBufOverwrite, NULL);
    data->SelectionStart = 0;
    data->SelectionEnd = 2;
    data->CursorPos = 0;
  }
  return 0;
}

typedef struct {
  int AddrDigitsCount;
  float LineHeight;
  float GlyphWidth;
  float HexCellWidth;
  float SpacingBetweenMidCols;
  float PosHexStart;
  float PosHexEnd;
  float PosAsciiStart;
  float PosAsciiEnd;
  float WindowWidth;
} Sizes;

const char *DataTypeGetDesc(ImGuiDataType data_type);
size_t DataTypeGetSize(ImGuiDataType data_type);

void meditDrawContents(GBMemoryEditor *editor, void *mem_data_void,
                       size_t mem_size, size_t base_display_addr);

void DrawPreviewLine(GBMemoryEditor *editor, Sizes *s, void *mem_data_void,
                     size_t mem_size, size_t base_display_addr);

void DrawOptionsLine(GBMemoryEditor *editor, Sizes *s, void *mem_data,
                     size_t mem_size, size_t base_display_addr);

void DrawPreviewData(GBMemoryEditor *editor, size_t addr, const ImU8 *mem_data,
                     size_t mem_size, ImGuiDataType data_type,
                     DataFormat data_format, char *out_buf,
                     size_t out_buf_size);

Sizes *meditNewSize() {
  Sizes *s = (Sizes *)malloc(sizeof(Sizes));
  memset(s, 0, sizeof(Sizes));
  return s;
}

void meditCalcSizes(GBMemoryEditor *editor, Sizes *s, size_t mem_size,
                    size_t base_display_addr) {
  ImGuiStyle *style = igGetStyle();
  s->AddrDigitsCount = editor->OptAddrDigitsCount;
  if (s->AddrDigitsCount == 0)
    for (size_t n = base_display_addr + mem_size - 1; n > 0; n >>= 4)
      s->AddrDigitsCount++;
  s->LineHeight = igGetTextLineHeight();
  ImVec2 pout;
  igCalcTextSize(&pout, "F", NULL, false, -1.0f);
  s->GlyphWidth = pout.x + 1; // We assume the font is mono-space
  s->HexCellWidth =
      (float)(int)(s->GlyphWidth *
                   2.5f); // "FF " we include trailing space in the width to
                          // easily catch clicks everywhere
  s->SpacingBetweenMidCols =
      (float)(int)(s->HexCellWidth * 0.25f); // Every OptMidColsCount columns we
                                             // add a bit of extra spacing
  s->PosHexStart = (s->AddrDigitsCount + 2) * s->GlyphWidth;
  s->PosHexEnd = s->PosHexStart + (s->HexCellWidth * editor->Cols);
  s->PosAsciiStart = s->PosAsciiEnd = s->PosHexEnd;
  if (editor->OptShowAscii) {
    s->PosAsciiStart = s->PosHexEnd + s->GlyphWidth * 1;
    if (editor->OptMidColsCount > 0)
      s->PosAsciiStart += (float)((editor->Cols + editor->OptMidColsCount - 1) /
                                  editor->OptMidColsCount) *
                          s->SpacingBetweenMidCols;
    s->PosAsciiEnd = s->PosAsciiStart + editor->Cols * s->GlyphWidth;
  }
  s->WindowWidth = s->PosAsciiEnd + style->ScrollbarSize +
                   style->WindowPadding.x * 2 + s->GlyphWidth;
}

GBMemoryEditor *meditMemoryEditorNew() {
  GBMemoryEditor *editor = (GBMemoryEditor *)malloc(sizeof(GBMemoryEditor));

  // Settings
  editor->Open = true;
  editor->ReadOnly = false;
  editor->Cols = 16;
  editor->OptShowOptions = true;
  editor->OptShowDataPreview = false;
  editor->OptShowHexII = false;
  editor->OptShowAscii = true;
  editor->OptGreyOutZeroes = true;
  editor->OptUpperCaseHex = true;
  editor->OptMidColsCount = 8;
  editor->OptAddrDigitsCount = 0;
  editor->HighlightColor = IM_COL32(255, 255, 255, 50);
  editor->ReadFn = NULL;
  editor->WriteFn = NULL;
  editor->HighlightFn = NULL;

  // State/Internals
  editor->ContentsWidthChanged = false;
  editor->DataPreviewAddr = editor->DataEditingAddr = (size_t)-1;
  editor->DataEditingTakeFocus = false;
  memset(editor->DataInputBuf, 0, sizeof(editor->DataInputBuf));
  memset(editor->AddrInputBuf, 0, sizeof(editor->AddrInputBuf));
  editor->GotoAddr = (size_t)-1;
  editor->HighlightMin = editor->HighlightMax = (size_t)-1;
  editor->PreviewEndianess = 0;
  editor->PreviewDataType = ImGuiDataType_S32;
  return editor;
}

void meditGotoAddrAndHighlight(GBMemoryEditor *editor, size_t addr_min,
                               size_t addr_max) {
  editor->GotoAddr = addr_min;
  editor->HighlightMin = addr_min;
  editor->HighlightMax = addr_max;
}

// Standalone Memory Editor window
void meditDrawWindow(GBMemoryEditor *editor, const char *title, void *mem_data,
                     size_t mem_size, size_t base_display_addr) {
  Sizes *s = meditNewSize();
  meditCalcSizes(editor, s, mem_size, base_display_addr);

  ImVec2 min;
  min.x = 0.0f;
  min.y = 0.0f;
  ImVec2 max;
  max.x = s->WindowWidth;
  max.y = FLT_MAX;

  igSetNextWindowSizeConstraints(min, max, NULL, NULL);

  editor->Open = true;
  if (igBegin(title, &editor->Open, ImGuiWindowFlags_NoScrollbar)) {
    if (igIsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
        igIsMouseReleased(ImGuiMouseButton_Right))
      igOpenPopup("context", 0);
    meditDrawContents(editor, mem_data, mem_size, base_display_addr);
    if (editor->ContentsWidthChanged) {
      meditCalcSizes(editor, s, mem_size, base_display_addr);
      ImVec2 size;
      igGetWindowSize(&size);
      size.x = s->WindowWidth;
      igSetWindowSizeVec2(size, 0);
    }
  }
  igEnd();
  free(s); // TODO
}

// Memory Editor contents only
void meditDrawContents(GBMemoryEditor *editor, void *mem_data_void,
                       size_t mem_size, size_t base_display_addr) {
  if (editor->Cols < 1)
    editor->Cols = 1;

  ImU8 *mem_data = (ImU8 *)mem_data_void;
  Sizes *s = meditNewSize(); // TODO
  meditCalcSizes(editor, s, mem_size, base_display_addr);
  ImGuiStyle *style = igGetStyle();

  // We begin into our scrolling region with the 'ImGuiWindowFlags_NoMove' in
  // order to prevent click from moving the window. This is used as a facility
  // since our main click detection code doesn't assign an ActiveId so the click
  // would normally be caught as a window-move.
  const float height_separator = style->ItemSpacing.y;
  float footer_height = 0;
  if (editor->OptShowOptions)
    footer_height += height_separator + igGetFrameHeightWithSpacing() * 1;
  if (editor->OptShowDataPreview)
    footer_height += height_separator + igGetFrameHeightWithSpacing() * 1 +
                     igGetTextLineHeightWithSpacing() * 3;

  ImVec2 size;
  size.x = 0;
  size.y = -footer_height;
  igBeginChildStr("##scrolling", size, false,
                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav);
  ImDrawList *draw_list = igGetWindowDrawList();

  ImVec2 padding;
  padding.x = 0;
  padding.y = 0;

  ImVec2 spacing;
  spacing.x = 0;
  spacing.y = 0;

  igPushStyleVarVec2(ImGuiStyleVar_FramePadding, padding);
  igPushStyleVarVec2(ImGuiStyleVar_ItemSpacing, spacing);

  // We are not really using the clipper API correctly here, because we rely on
  // visible_start_addr/visible_end_addr for our scrolling function.
  const int line_total_count =
      (int)((mem_size + editor->Cols - 1) / editor->Cols);
  ImGuiListClipper *clipper = ImGuiListClipper_ImGuiListClipper(); // TODO
  ImGuiListClipper_Begin(clipper, line_total_count, s->LineHeight);
  ImGuiListClipper_Step(clipper);
  const size_t visible_start_addr = clipper->DisplayStart * editor->Cols;
  const size_t visible_end_addr = clipper->DisplayEnd * editor->Cols;

  bool data_next = false;

  if (editor->ReadOnly || editor->DataEditingAddr >= mem_size)
    editor->DataEditingAddr = (size_t)-1;
  if (editor->DataPreviewAddr >= mem_size)
    editor->DataPreviewAddr = (size_t)-1;

  size_t preview_data_type_size =
      editor->OptShowDataPreview ? DataTypeGetSize(editor->PreviewDataType) : 0;

  size_t data_editing_addr_backup = editor->DataEditingAddr;
  size_t data_editing_addr_next = (size_t)-1;
  if (editor->DataEditingAddr != (size_t)-1) {
    // Move cursor but only apply on next frame so scrolling with be
    // synchronized (because currently we can't change the scrolling while the
    // window is being rendered)
    if (igIsKeyPressed(igGetKeyIndex(ImGuiKey_UpArrow), true) &&
        editor->DataEditingAddr >= (size_t)editor->Cols) {
      data_editing_addr_next = editor->DataEditingAddr - editor->Cols;
      editor->DataEditingTakeFocus = true;
    } else if (igIsKeyPressed(igGetKeyIndex(ImGuiKey_DownArrow), true) &&
               editor->DataEditingAddr < mem_size - editor->Cols) {
      data_editing_addr_next = editor->DataEditingAddr + editor->Cols;
      editor->DataEditingTakeFocus = true;
    } else if (igIsKeyPressed(igGetKeyIndex(ImGuiKey_LeftArrow), true) &&
               editor->DataEditingAddr > 0) {
      data_editing_addr_next = editor->DataEditingAddr - 1;
      editor->DataEditingTakeFocus = true;
    } else if (igIsKeyPressed(igGetKeyIndex(ImGuiKey_RightArrow), true) &&
               editor->DataEditingAddr < mem_size - 1) {
      data_editing_addr_next = editor->DataEditingAddr + 1;
      editor->DataEditingTakeFocus = true;
    }
  }
  if (data_editing_addr_next != (size_t)-1 &&
      (data_editing_addr_next / editor->Cols) !=
          (data_editing_addr_backup / editor->Cols)) {
    // Track cursor movements
    const int scroll_offset = ((int)(data_editing_addr_next / editor->Cols) -
                               (int)(data_editing_addr_backup / editor->Cols));
    const bool scroll_desired =
        (scroll_offset < 0 &&
         data_editing_addr_next < visible_start_addr + editor->Cols * 2) ||
        (scroll_offset > 0 &&
         data_editing_addr_next > visible_end_addr - editor->Cols * 2);
    if (scroll_desired)
      igSetScrollYFloat(igGetScrollY() + scroll_offset * s->LineHeight);
  }

  // Draw vertical separator
  ImVec2 window_pos;
  igGetWindowPos(&window_pos);
  if (editor->OptShowAscii) {
    ImVec2 p1;
    p1.x = window_pos.x + s->PosAsciiStart - s->GlyphWidth;
    p1.y = window_pos.y;

    ImVec2 p2;
    p2.x = window_pos.x + s->PosAsciiStart - s->GlyphWidth;
    p2.y = window_pos.y + 9999;

    ImDrawList_AddLine(
        draw_list, p1, p2,
        igGetColorU32U32(ImGuiCol_Border), 1.0f);
  }

  const ImU32 color_text = igGetColorU32U32(ImGuiCol_Text);
  const ImU32 color_disabled = editor->OptGreyOutZeroes
                                   ? igGetColorU32U32(ImGuiCol_TextDisabled)
                                   : color_text;

  const char *format_address =
      editor->OptUpperCaseHex ? "%0*" _PRISizeT "X: " : "%0*" _PRISizeT "x: ";
  const char *format_data =
      editor->OptUpperCaseHex ? "%0*" _PRISizeT "X" : "%0*" _PRISizeT "x";
  const char *format_byte = editor->OptUpperCaseHex ? "%02X" : "%02x";
  const char *format_byte_space = editor->OptUpperCaseHex ? "%02X " : "%02x ";

  for (int line_i = clipper->DisplayStart; line_i < clipper->DisplayEnd;
       line_i++) // display only visible lines
  {
    size_t addr = (size_t)(line_i * editor->Cols);
    igText(format_address, s->AddrDigitsCount, base_display_addr + addr);

    // Draw Hexadecimal
    for (int n = 0; n < editor->Cols && addr < mem_size; n++, addr++) {
      float byte_pos_x = s->PosHexStart + s->HexCellWidth * n;
      if (editor->OptMidColsCount > 0)
        byte_pos_x +=
            (float)(n / editor->OptMidColsCount) * s->SpacingBetweenMidCols;
      igSameLine(byte_pos_x, -1.0f);

      // Draw highlight
      bool is_highlight_from_user_range =
          (addr >= editor->HighlightMin && addr < editor->HighlightMax);
      bool is_highlight_from_user_func =
          (editor->HighlightFn && editor->HighlightFn(mem_data, addr));
      bool is_highlight_from_preview =
          (addr >= editor->DataPreviewAddr &&
           addr < editor->DataPreviewAddr + preview_data_type_size);
      if (is_highlight_from_user_range || is_highlight_from_user_func ||
          is_highlight_from_preview) {
        ImVec2 pos;
        igGetCursorScreenPos(&pos);
        float highlight_width = s->GlyphWidth * 2;
        bool is_next_byte_highlighted =
            (addr + 1 < mem_size) &&
            ((editor->HighlightMax != (size_t)-1 &&
              addr + 1 < editor->HighlightMax) ||
             (editor->HighlightFn && editor->HighlightFn(mem_data, addr + 1)));
        if (is_next_byte_highlighted || (n + 1 == editor->Cols)) {
          highlight_width = s->HexCellWidth;
          if (editor->OptMidColsCount > 0 && n > 0 && (n + 1) < editor->Cols &&
              ((n + 1) % editor->OptMidColsCount) == 0)
            highlight_width += s->SpacingBetweenMidCols;
        }
        ImVec2 pmax;
        pmax.x = pos.x + highlight_width;
        pmax.y = pos.y + s->LineHeight;
        ImDrawList_AddRectFilled(
            draw_list, pos, pmax,
            editor->HighlightColor, 0.f, 15);
      }

      if (editor->DataEditingAddr == addr) {
        // Display text input on current byte
        bool data_write = false;
        igPushIDPtr((void *)addr); // TODO
        if (editor->DataEditingTakeFocus) {
          igSetKeyboardFocusHere(0);
          igCaptureKeyboardFromApp(true);
          sprintf(editor->AddrInputBuf, format_data, s->AddrDigitsCount,
                  base_display_addr + addr);
          sprintf(editor->DataInputBuf, format_byte,
                  editor->ReadFn ? editor->ReadFn(mem_data, addr)
                                 : mem_data[addr]);
        }
        igPushItemWidth(s->GlyphWidth * 2);
        GBMemoryEditorUserData user_data;
        user_data.CursorPos = -1;
        sprintf(user_data.CurrentBufOverwrite, format_byte,
                editor->ReadFn ? editor->ReadFn(mem_data, addr)
                               : mem_data[addr]);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsHexadecimal |
                                    ImGuiInputTextFlags_EnterReturnsTrue |
                                    ImGuiInputTextFlags_AutoSelectAll |
                                    ImGuiInputTextFlags_NoHorizontalScroll |
                                    ImGuiInputTextFlags_AlwaysInsertMode |
                                    ImGuiInputTextFlags_CallbackAlways;
        if (igInputText("##data", editor->DataInputBuf, 32, flags,
                        GBMemoryEditorUserDataCallback, &user_data))
          data_write = data_next = true;
        else if (!editor->DataEditingTakeFocus && !igIsItemActive())
          editor->DataEditingAddr = data_editing_addr_next = (size_t)-1;
        editor->DataEditingTakeFocus = false;
        igPopItemWidth();
        if (user_data.CursorPos >= 2)
          data_write = data_next = true;
        if (data_editing_addr_next != (size_t)-1)
          data_write = data_next = false;
        unsigned int data_input_value = 0;
        if (data_write &&
            sscanf(editor->DataInputBuf, "%X", &data_input_value) == 1) {
          if (editor->WriteFn)
            editor->WriteFn(mem_data, addr, (ImU8)data_input_value);
          else
            mem_data[addr] = (ImU8)data_input_value;
        }
        igPopID();
      } else {
        // NB: The trailing space is not visible but ensure there's no gap that
        // the mouse cannot click on.
        ImU8 b =
            editor->ReadFn ? editor->ReadFn(mem_data, addr) : mem_data[addr];

        if (editor->OptShowHexII) {
          if ((b >= 32 && b < 128))
            igText(".%c ", b);
          else if (b == 0xFF && editor->OptGreyOutZeroes)
            igTextDisabled("## ");
          else if (b == 0x00)
            igText("   ");
          else
            igText(format_byte_space, b);
        } else {
          if (b == 0 && editor->OptGreyOutZeroes)
            igTextDisabled("00 ");
          else
            igText(format_byte_space, b);
        }
        if (!editor->ReadOnly && igIsItemHovered(0) &&
            igIsMouseClicked(0, false)) {
          editor->DataEditingTakeFocus = true;
          data_editing_addr_next = addr;
        }
      }
    }

    if (editor->OptShowAscii) {
      // Draw ASCII values
      igSameLine(s->PosAsciiStart, -1.0f);
      ImVec2 pos;
      igGetCursorScreenPos(&pos);
      addr = line_i * editor->Cols;
      igPushIDInt(line_i); // TODO

      ImVec2 size;
      size.x = s->PosAsciiEnd - s->PosAsciiStart;
      size.y = s->LineHeight;

      if (igInvisibleButton(
              "ascii", size,
              0)) {
        editor->DataEditingAddr = editor->DataPreviewAddr =
            addr + (size_t)((igGetIO()->MousePos.x - pos.x) / s->GlyphWidth);
        editor->DataEditingTakeFocus = true;
      }
      igPopID();
      for (int n = 0; n < editor->Cols && addr < mem_size; n++, addr++) {
        if (addr == editor->DataEditingAddr) {
          ImVec2 pmax;
          pmax.x = pos.x + s->GlyphWidth;
          pmax.y = pos.y + s->LineHeight;
          ImDrawList_AddRectFilled(
              draw_list, pos,
              pmax,
              igGetColorU32U32(ImGuiCol_FrameBg), 0.f, 15);
          ImDrawList_AddRectFilled(
              draw_list, pos,
              pmax,
              igGetColorU32U32(ImGuiCol_TextSelectedBg), 0.f, 15);
        }
        unsigned char c =
            editor->ReadFn ? editor->ReadFn(mem_data, addr) : mem_data[addr];
        char display_c = (c < 32 || c >= 128) ? '.' : c;
        ImDrawList_AddTextVec2(draw_list, pos,
                               (display_c == c) ? color_text : color_disabled,
                               &display_c, &display_c + 1);
        pos.x += s->GlyphWidth;
      }
    }
  }
  IM_ASSERT(ImGuiListClipper_Step(clipper) == false);
  ImGuiListClipper_End(clipper);
  ImGuiListClipper_destroy(clipper); // TODO
  igPopStyleVar(2);
  igEndChild();

  if (data_next && editor->DataEditingAddr < mem_size) {
    editor->DataEditingAddr = editor->DataPreviewAddr =
        editor->DataEditingAddr + 1;
    editor->DataEditingTakeFocus = true;
  } else if (data_editing_addr_next != (size_t)-1) {
    editor->DataEditingAddr = editor->DataPreviewAddr = data_editing_addr_next;
  }

  const bool lock_show_data_preview = editor->OptShowDataPreview;
  if (editor->OptShowOptions) {
    igSeparator();
    DrawOptionsLine(editor, s, mem_data, mem_size, base_display_addr);
  }

  if (lock_show_data_preview) {
    igSeparator();
    DrawPreviewLine(editor, s, mem_data, mem_size, base_display_addr);
  }

  // Notify the main window of our ideal child content size (FIXME: we are
  // missing an API to get the contents size from the child)
  igSetCursorPosX(s->WindowWidth);
}

void DrawOptionsLine(GBMemoryEditor *editor, Sizes *s, void *mem_data,
                     size_t mem_size, size_t base_display_addr) {
  IM_UNUSED(mem_data);
  ImGuiStyle *style = igGetStyle();
  const char *format_range = editor->OptUpperCaseHex
                                 ? "Range %0*" _PRISizeT "X..%0*" _PRISizeT "X"
                                 : "Range %0*" _PRISizeT "x..%0*" _PRISizeT "x";

  ImVec2 btn;
  btn.x = 0;
  btn.y = 0;

  // Options menu
  if (igButton("Options", btn))
    igOpenPopup("context", 0);
  if (igBeginPopup("context", 0)) {
    igPushItemWidth(56);
    if (igDragInt("##cols", &editor->Cols, 0.2f, 4, 32, "%d cols", 0)) {
      editor->ContentsWidthChanged = true;
      if (editor->Cols < 1)
        editor->Cols = 1;
    }
    igPopItemWidth();
    igCheckbox("Show Data Preview", &editor->OptShowDataPreview);
    igCheckbox("Show HexII", &editor->OptShowHexII);
    if (igCheckbox("Show Ascii", &editor->OptShowAscii)) {
      editor->ContentsWidthChanged = true;
    }
    igCheckbox("Grey out zeroes", &editor->OptGreyOutZeroes);
    igCheckbox("Uppercase Hex", &editor->OptUpperCaseHex);

    igEndPopup();
  }

  igSameLine(0.0f, 1.0f);
  igText(format_range, s->AddrDigitsCount, base_display_addr,
         s->AddrDigitsCount, base_display_addr + mem_size - 1);
  igSameLine(0.0f, 1.0f);
  igPushItemWidth((s->AddrDigitsCount + 1) * s->GlyphWidth +
                  style->FramePadding.x * 2.0f);
  if (igInputText("##addr", editor->AddrInputBuf, 32,
                  ImGuiInputTextFlags_CharsHexadecimal |
                      ImGuiInputTextFlags_EnterReturnsTrue,
                  NULL, NULL)) {
    size_t goto_addr;
    if (sscanf(editor->AddrInputBuf, "%" _PRISizeT "X", &goto_addr) == 1) {
      editor->GotoAddr = goto_addr - base_display_addr;
      editor->HighlightMin = editor->HighlightMax = (size_t)-1;
    }
  }
  igPopItemWidth();

  if (editor->GotoAddr != (size_t)-1) {
    if (editor->GotoAddr < mem_size) {
      ImVec2 child;
      child.x = 0;
      child.y = 0;
      igBeginChildStr("##scrolling", child, false, 0);
      ImVec2 startPos;
      igGetCursorStartPos(&startPos);
      igSetScrollFromPosYFloat(startPos.y + (editor->GotoAddr / editor->Cols) *
                                                igGetTextLineHeight(),
                               0.5f);
      igEndChild();
      editor->DataEditingAddr = editor->DataPreviewAddr = editor->GotoAddr;
      editor->DataEditingTakeFocus = true;
    }
    editor->GotoAddr = (size_t)-1;
  }
}

void DrawPreviewLine(GBMemoryEditor *editor, Sizes *s, void *mem_data_void,
                     size_t mem_size, size_t base_display_addr) {
  IM_UNUSED(base_display_addr);
  ImU8 *mem_data = (ImU8 *)mem_data_void;
  ImGuiStyle *style = igGetStyle();
  igAlignTextToFramePadding();
  igText("Preview as:");
  igSameLine(0.0f, 1.0f);
  igPushItemWidth((s->GlyphWidth * 10.0f) + style->FramePadding.x * 2.0f +
                  style->ItemInnerSpacing.x);
  if (igBeginCombo("##combo_type", DataTypeGetDesc(editor->PreviewDataType),
                   ImGuiComboFlags_HeightLargest)) {
    ImVec2 selectable;
    selectable.x = 0;
    selectable.y = 0;
    for (int n = 0; n < ImGuiDataType_COUNT; n++)
      if (igSelectableBool(DataTypeGetDesc((ImGuiDataType)n),
                           editor->PreviewDataType == n, 0, selectable))
        editor->PreviewDataType = (ImGuiDataType)n;
    igEndCombo();
  }
  igPopItemWidth();
  igSameLine(0.0f, 1.0f);
  igPushItemWidth((s->GlyphWidth * 6.0f) + style->FramePadding.x * 2.0f +
                  style->ItemInnerSpacing.x);
  igComboStr("##combo_endianess", &editor->PreviewEndianess, "LE\0BE\0\0", -1);
  igPopItemWidth();

  char buf[128] = "";
  float x = s->GlyphWidth * 6.0f;
  bool has_value = editor->DataPreviewAddr != (size_t)-1;
  if (has_value)
    DrawPreviewData(editor, editor->DataPreviewAddr, mem_data, mem_size,
                    editor->PreviewDataType, DataFormat_Dec, buf,
                    (size_t)IM_ARRAYSIZE(buf));
  igText("Dec");
  igSameLine(x, 1.0f);
  igTextUnformatted(has_value ? buf : "N/A", NULL);
  if (has_value)
    DrawPreviewData(editor, editor->DataPreviewAddr, mem_data, mem_size,
                    editor->PreviewDataType, DataFormat_Hex, buf,
                    (size_t)IM_ARRAYSIZE(buf));
  igText("Hex");
  igSameLine(x, 1.0f);
  igTextUnformatted(has_value ? buf : "N/A", NULL);
  if (has_value)
    DrawPreviewData(editor, editor->DataPreviewAddr, mem_data, mem_size,
                    editor->PreviewDataType, DataFormat_Bin, buf,
                    (size_t)IM_ARRAYSIZE(buf));
  buf[IM_ARRAYSIZE(buf) - 1] = 0;
  igText("Bin");
  igSameLine(x, 1.0f);
  igTextUnformatted(has_value ? buf : "N/A", NULL);
}

// Utilities for Data Preview
const char *DataTypeGetDesc(ImGuiDataType data_type) {
  const char *descs[] = {"Int8",   "Uint8", "Int16",  "Uint16", "Int32",
                         "Uint32", "Int64", "Uint64", "Float",  "Double"};
  IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
  return descs[data_type];
}

size_t DataTypeGetSize(ImGuiDataType data_type) {
  const size_t sizes[] = {
      1, 1, 2, 2, 4, 4, 8, 8, sizeof(float), sizeof(double)};
  IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
  return sizes[data_type];
}

const char *DataFormatGetDesc(DataFormat data_format) {
  const char *descs[] = {"Bin", "Dec", "Hex"};
  IM_ASSERT(data_format >= 0 && data_format < DataFormat_COUNT);
  return descs[data_format];
}

bool IsBigEndian() {
  uint16_t x = 1;
  char c[2];
  memcpy(c, &x, 2);
  return c[0] != 0;
}

static void *EndianessCopyBigEndian(void *_dst, void *_src, size_t s,
                                    int is_little_endian) {
  if (is_little_endian) {
    uint8_t *dst = (uint8_t *)_dst;
    uint8_t *src = (uint8_t *)_src + s - 1;
    for (int i = 0, n = (int)s; i < n; ++i)
      memcpy(dst++, src--, 1);
    return _dst;
  } else {
    return memcpy(_dst, _src, s);
  }
}

static void *EndianessCopyLittleEndian(void *_dst, void *_src, size_t s,
                                       int is_little_endian) {
  if (is_little_endian) {
    return memcpy(_dst, _src, s);
  } else {
    uint8_t *dst = (uint8_t *)_dst;
    uint8_t *src = (uint8_t *)_src + s - 1;
    for (int i = 0, n = (int)s; i < n; ++i)
      memcpy(dst++, src--, 1);
    return _dst;
  }
}

void *EndianessCopy(GBMemoryEditor *editor, void *dst, void *src, size_t size) {
  static void *(*fp)(void *, void *, size_t, int) = NULL;
  if (fp == NULL)
    fp = IsBigEndian() ? EndianessCopyBigEndian : EndianessCopyLittleEndian;
  return fp(dst, src, size, editor->PreviewEndianess);
}

const char *FormatBinary(const uint8_t *buf, int width) {
  IM_ASSERT(width <= 64);
  size_t out_n = 0;
  static char out_buf[64 + 8 + 1];
  int n = width / 8;
  for (int j = n - 1; j >= 0; --j) {
    for (int i = 0; i < 8; ++i)
      out_buf[out_n++] = (buf[j] & (1 << (7 - i))) ? '1' : '0';
    out_buf[out_n++] = ' ';
  }
  IM_ASSERT(out_n < IM_ARRAYSIZE(out_buf));
  out_buf[out_n] = 0;
  return out_buf;
}

// [Internal]
void DrawPreviewData(GBMemoryEditor *editor, size_t addr, const ImU8 *mem_data,
                     size_t mem_size, ImGuiDataType data_type,
                     DataFormat data_format, char *out_buf,
                     size_t out_buf_size) {
  uint8_t buf[8];
  size_t elem_size = DataTypeGetSize(data_type);
  size_t size = addr + elem_size > mem_size ? mem_size - addr : elem_size;
  if (editor->ReadFn)
    for (int i = 0, n = (int)size; i < n; ++i)
      buf[i] = editor->ReadFn(mem_data, addr + i);
  else
    memcpy(buf, mem_data + addr, size);

  if (data_format == DataFormat_Bin) {
    uint8_t binbuf[8];
    EndianessCopy(editor, binbuf, buf, size);
    ImSnprintf(out_buf, out_buf_size, "%s",
               FormatBinary(binbuf, (int)size * 8));
    return;
  }

  out_buf[0] = 0;
  switch (data_type) {
  case ImGuiDataType_S8: {
    int8_t int8 = 0;
    EndianessCopy(editor, &int8, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%hhd", int8);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%02x", int8 & 0xFF);
      return;
    }
    break;
  }
  case ImGuiDataType_U8: {
    uint8_t uint8 = 0;
    EndianessCopy(editor, &uint8, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%hhu", uint8);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%02x", uint8 & 0XFF);
      return;
    }
    break;
  }
  case ImGuiDataType_S16: {
    int16_t int16 = 0;
    EndianessCopy(editor, &int16, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%hd", int16);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%04x", int16 & 0xFFFF);
      return;
    }
    break;
  }
  case ImGuiDataType_U16: {
    uint16_t uint16 = 0;
    EndianessCopy(editor, &uint16, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%hu", uint16);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%04x", uint16 & 0xFFFF);
      return;
    }
    break;
  }
  case ImGuiDataType_S32: {
    int32_t int32 = 0;
    EndianessCopy(editor, &int32, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%d", int32);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%08x", int32);
      return;
    }
    break;
  }
  case ImGuiDataType_U32: {
    uint32_t uint32 = 0;
    EndianessCopy(editor, &uint32, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%u", uint32);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%08x", uint32);
      return;
    }
    break;
  }
  case ImGuiDataType_S64: {
    int64_t int64 = 0;
    EndianessCopy(editor, &int64, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%lld", (long long)int64);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%016llx", (long long)int64);
      return;
    }
    break;
  }
  case ImGuiDataType_U64: {
    uint64_t uint64 = 0;
    EndianessCopy(editor, &uint64, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%llu", (long long)uint64);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "0x%016llx", (long long)uint64);
      return;
    }
    break;
  }
  case ImGuiDataType_Float: {
    float float32 = 0.0f;
    EndianessCopy(editor, &float32, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%f", float32);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "%a", float32);
      return;
    }
    break;
  }
  case ImGuiDataType_Double: {
    double float64 = 0.0;
    EndianessCopy(editor, &float64, buf, size);
    if (data_format == DataFormat_Dec) {
      ImSnprintf(out_buf, out_buf_size, "%f", float64);
      return;
    }
    if (data_format == DataFormat_Hex) {
      ImSnprintf(out_buf, out_buf_size, "%a", float64);
      return;
    }
    break;
  }
  case ImGuiDataType_COUNT:
    break;
  }             // Switch
  IM_ASSERT(0); // Shouldn't reach
}

#undef _PRISizeT
#undef ImSnprintf

#ifdef _MSC_VER
#pragma warning(pop)
#endif