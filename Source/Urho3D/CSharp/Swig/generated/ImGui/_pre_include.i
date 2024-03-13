%rename(ImGuiWindowFlags) ImGuiWindowFlags_;
%typemap(csattributes) ImGuiWindowFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiWindowFlags_None;
%csconstvalue("1") ImGuiWindowFlags_NoTitleBar;
%csconstvalue("2") ImGuiWindowFlags_NoResize;
%csconstvalue("4") ImGuiWindowFlags_NoMove;
%csconstvalue("8") ImGuiWindowFlags_NoScrollbar;
%csconstvalue("16") ImGuiWindowFlags_NoScrollWithMouse;
%csconstvalue("32") ImGuiWindowFlags_NoCollapse;
%csconstvalue("64") ImGuiWindowFlags_AlwaysAutoResize;
%csconstvalue("128") ImGuiWindowFlags_NoBackground;
%csconstvalue("256") ImGuiWindowFlags_NoSavedSettings;
%csconstvalue("512") ImGuiWindowFlags_NoMouseInputs;
%csconstvalue("1024") ImGuiWindowFlags_MenuBar;
%csconstvalue("2048") ImGuiWindowFlags_HorizontalScrollbar;
%csconstvalue("4096") ImGuiWindowFlags_NoFocusOnAppearing;
%csconstvalue("8192") ImGuiWindowFlags_NoBringToFrontOnFocus;
%csconstvalue("16384") ImGuiWindowFlags_AlwaysVerticalScrollbar;
%csconstvalue("32768") ImGuiWindowFlags_AlwaysHorizontalScrollbar;
%csconstvalue("65536") ImGuiWindowFlags_AlwaysUseWindowPadding;
%csconstvalue("262144") ImGuiWindowFlags_NoNavInputs;
%csconstvalue("524288") ImGuiWindowFlags_NoNavFocus;
%csconstvalue("1048576") ImGuiWindowFlags_UnsavedDocument;
%csconstvalue("2097152") ImGuiWindowFlags_NoDocking;
%csconstvalue("786432") ImGuiWindowFlags_NoNav;
%csconstvalue("43") ImGuiWindowFlags_NoDecoration;
%csconstvalue("786944") ImGuiWindowFlags_NoInputs;
%csconstvalue("8388608") ImGuiWindowFlags_NavFlattened;
%csconstvalue("16777216") ImGuiWindowFlags_ChildWindow;
%csconstvalue("33554432") ImGuiWindowFlags_Tooltip;
%csconstvalue("67108864") ImGuiWindowFlags_Popup;
%csconstvalue("134217728") ImGuiWindowFlags_Modal;
%csconstvalue("268435456") ImGuiWindowFlags_ChildMenu;
%csconstvalue("536870912") ImGuiWindowFlags_DockNodeHost;
%rename(ImGuiInputTextFlags) ImGuiInputTextFlags_;
%typemap(csattributes) ImGuiInputTextFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiInputTextFlags_None;
%csconstvalue("1") ImGuiInputTextFlags_CharsDecimal;
%csconstvalue("2") ImGuiInputTextFlags_CharsHexadecimal;
%csconstvalue("4") ImGuiInputTextFlags_CharsUppercase;
%csconstvalue("8") ImGuiInputTextFlags_CharsNoBlank;
%csconstvalue("16") ImGuiInputTextFlags_AutoSelectAll;
%csconstvalue("32") ImGuiInputTextFlags_EnterReturnsTrue;
%csconstvalue("64") ImGuiInputTextFlags_CallbackCompletion;
%csconstvalue("128") ImGuiInputTextFlags_CallbackHistory;
%csconstvalue("256") ImGuiInputTextFlags_CallbackAlways;
%csconstvalue("512") ImGuiInputTextFlags_CallbackCharFilter;
%csconstvalue("1024") ImGuiInputTextFlags_AllowTabInput;
%csconstvalue("2048") ImGuiInputTextFlags_CtrlEnterForNewLine;
%csconstvalue("4096") ImGuiInputTextFlags_NoHorizontalScroll;
%csconstvalue("8192") ImGuiInputTextFlags_AlwaysOverwrite;
%csconstvalue("16384") ImGuiInputTextFlags_ReadOnly;
%csconstvalue("32768") ImGuiInputTextFlags_Password;
%csconstvalue("65536") ImGuiInputTextFlags_NoUndoRedo;
%csconstvalue("131072") ImGuiInputTextFlags_CharsScientific;
%csconstvalue("262144") ImGuiInputTextFlags_CallbackResize;
%csconstvalue("524288") ImGuiInputTextFlags_CallbackEdit;
%csconstvalue("8192") ImGuiInputTextFlags_AlwaysInsertMode;
%rename(ImGuiTreeNodeFlags) ImGuiTreeNodeFlags_;
%typemap(csattributes) ImGuiTreeNodeFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiTreeNodeFlags_None;
%csconstvalue("1") ImGuiTreeNodeFlags_Selected;
%csconstvalue("2") ImGuiTreeNodeFlags_Framed;
%csconstvalue("4") ImGuiTreeNodeFlags_AllowItemOverlap;
%csconstvalue("8") ImGuiTreeNodeFlags_NoTreePushOnOpen;
%csconstvalue("16") ImGuiTreeNodeFlags_NoAutoOpenOnLog;
%csconstvalue("32") ImGuiTreeNodeFlags_DefaultOpen;
%csconstvalue("64") ImGuiTreeNodeFlags_OpenOnDoubleClick;
%csconstvalue("128") ImGuiTreeNodeFlags_OpenOnArrow;
%csconstvalue("256") ImGuiTreeNodeFlags_Leaf;
%csconstvalue("512") ImGuiTreeNodeFlags_Bullet;
%csconstvalue("1024") ImGuiTreeNodeFlags_FramePadding;
%csconstvalue("2048") ImGuiTreeNodeFlags_SpanAvailWidth;
%csconstvalue("4096") ImGuiTreeNodeFlags_SpanFullWidth;
%csconstvalue("8192") ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
%csconstvalue("26") ImGuiTreeNodeFlags_CollapsingHeader;
%rename(ImGuiPopupFlags) ImGuiPopupFlags_;
%typemap(csattributes) ImGuiPopupFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiPopupFlags_None;
%csconstvalue("0") ImGuiPopupFlags_MouseButtonLeft;
%csconstvalue("1") ImGuiPopupFlags_MouseButtonRight;
%csconstvalue("2") ImGuiPopupFlags_MouseButtonMiddle;
%csconstvalue("31") ImGuiPopupFlags_MouseButtonMask_;
%csconstvalue("1") ImGuiPopupFlags_MouseButtonDefault_;
%csconstvalue("32") ImGuiPopupFlags_NoOpenOverExistingPopup;
%csconstvalue("64") ImGuiPopupFlags_NoOpenOverItems;
%csconstvalue("128") ImGuiPopupFlags_AnyPopupId;
%csconstvalue("256") ImGuiPopupFlags_AnyPopupLevel;
%csconstvalue("384") ImGuiPopupFlags_AnyPopup;
%rename(ImGuiSelectableFlags) ImGuiSelectableFlags_;
%typemap(csattributes) ImGuiSelectableFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiSelectableFlags_None;
%csconstvalue("1") ImGuiSelectableFlags_DontClosePopups;
%csconstvalue("2") ImGuiSelectableFlags_SpanAllColumns;
%csconstvalue("4") ImGuiSelectableFlags_AllowDoubleClick;
%csconstvalue("8") ImGuiSelectableFlags_Disabled;
%csconstvalue("16") ImGuiSelectableFlags_AllowItemOverlap;
%rename(ImGuiComboFlags) ImGuiComboFlags_;
%typemap(csattributes) ImGuiComboFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiComboFlags_None;
%csconstvalue("1") ImGuiComboFlags_PopupAlignLeft;
%csconstvalue("2") ImGuiComboFlags_HeightSmall;
%csconstvalue("4") ImGuiComboFlags_HeightRegular;
%csconstvalue("8") ImGuiComboFlags_HeightLarge;
%csconstvalue("16") ImGuiComboFlags_HeightLargest;
%csconstvalue("32") ImGuiComboFlags_NoArrowButton;
%csconstvalue("64") ImGuiComboFlags_NoPreview;
%csconstvalue("30") ImGuiComboFlags_HeightMask_;
%rename(ImGuiTabBarFlags) ImGuiTabBarFlags_;
%typemap(csattributes) ImGuiTabBarFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiTabBarFlags_None;
%csconstvalue("1") ImGuiTabBarFlags_Reorderable;
%csconstvalue("2") ImGuiTabBarFlags_AutoSelectNewTabs;
%csconstvalue("4") ImGuiTabBarFlags_TabListPopupButton;
%csconstvalue("8") ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
%csconstvalue("16") ImGuiTabBarFlags_NoTabListScrollingButtons;
%csconstvalue("32") ImGuiTabBarFlags_NoTooltip;
%csconstvalue("64") ImGuiTabBarFlags_FittingPolicyResizeDown;
%csconstvalue("128") ImGuiTabBarFlags_FittingPolicyScroll;
%csconstvalue("192") ImGuiTabBarFlags_FittingPolicyMask_;
%csconstvalue("64") ImGuiTabBarFlags_FittingPolicyDefault_;
%rename(ImGuiTabItemFlags) ImGuiTabItemFlags_;
%typemap(csattributes) ImGuiTabItemFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiTabItemFlags_None;
%csconstvalue("1") ImGuiTabItemFlags_UnsavedDocument;
%csconstvalue("2") ImGuiTabItemFlags_SetSelected;
%csconstvalue("4") ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
%csconstvalue("8") ImGuiTabItemFlags_NoPushId;
%csconstvalue("16") ImGuiTabItemFlags_NoTooltip;
%csconstvalue("32") ImGuiTabItemFlags_NoReorder;
%csconstvalue("64") ImGuiTabItemFlags_Leading;
%csconstvalue("128") ImGuiTabItemFlags_Trailing;
%rename(ImGuiTableFlags) ImGuiTableFlags_;
%typemap(csattributes) ImGuiTableFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiTableFlags_None;
%csconstvalue("1") ImGuiTableFlags_Resizable;
%csconstvalue("2") ImGuiTableFlags_Reorderable;
%csconstvalue("4") ImGuiTableFlags_Hideable;
%csconstvalue("8") ImGuiTableFlags_Sortable;
%csconstvalue("16") ImGuiTableFlags_NoSavedSettings;
%csconstvalue("32") ImGuiTableFlags_ContextMenuInBody;
%csconstvalue("64") ImGuiTableFlags_RowBg;
%csconstvalue("128") ImGuiTableFlags_BordersInnerH;
%csconstvalue("256") ImGuiTableFlags_BordersOuterH;
%csconstvalue("512") ImGuiTableFlags_BordersInnerV;
%csconstvalue("1024") ImGuiTableFlags_BordersOuterV;
%csconstvalue("384") ImGuiTableFlags_BordersH;
%csconstvalue("1536") ImGuiTableFlags_BordersV;
%csconstvalue("640") ImGuiTableFlags_BordersInner;
%csconstvalue("1280") ImGuiTableFlags_BordersOuter;
%csconstvalue("1920") ImGuiTableFlags_Borders;
%csconstvalue("2048") ImGuiTableFlags_NoBordersInBody;
%csconstvalue("4096") ImGuiTableFlags_NoBordersInBodyUntilResize;
%csconstvalue("8192") ImGuiTableFlags_SizingFixedFit;
%csconstvalue("16384") ImGuiTableFlags_SizingFixedSame;
%csconstvalue("24576") ImGuiTableFlags_SizingStretchProp;
%csconstvalue("32768") ImGuiTableFlags_SizingStretchSame;
%csconstvalue("65536") ImGuiTableFlags_NoHostExtendX;
%csconstvalue("131072") ImGuiTableFlags_NoHostExtendY;
%csconstvalue("262144") ImGuiTableFlags_NoKeepColumnsVisible;
%csconstvalue("524288") ImGuiTableFlags_PreciseWidths;
%csconstvalue("1048576") ImGuiTableFlags_NoClip;
%csconstvalue("2097152") ImGuiTableFlags_PadOuterX;
%csconstvalue("4194304") ImGuiTableFlags_NoPadOuterX;
%csconstvalue("8388608") ImGuiTableFlags_NoPadInnerX;
%csconstvalue("16777216") ImGuiTableFlags_ScrollX;
%csconstvalue("33554432") ImGuiTableFlags_ScrollY;
%csconstvalue("67108864") ImGuiTableFlags_SortMulti;
%csconstvalue("134217728") ImGuiTableFlags_SortTristate;
%csconstvalue("57344") ImGuiTableFlags_SizingMask_;
%rename(ImGuiTableColumnFlags) ImGuiTableColumnFlags_;
%typemap(csattributes) ImGuiTableColumnFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiTableColumnFlags_None;
%csconstvalue("1") ImGuiTableColumnFlags_Disabled;
%csconstvalue("2") ImGuiTableColumnFlags_DefaultHide;
%csconstvalue("4") ImGuiTableColumnFlags_DefaultSort;
%csconstvalue("8") ImGuiTableColumnFlags_WidthStretch;
%csconstvalue("16") ImGuiTableColumnFlags_WidthFixed;
%csconstvalue("32") ImGuiTableColumnFlags_NoResize;
%csconstvalue("64") ImGuiTableColumnFlags_NoReorder;
%csconstvalue("128") ImGuiTableColumnFlags_NoHide;
%csconstvalue("256") ImGuiTableColumnFlags_NoClip;
%csconstvalue("512") ImGuiTableColumnFlags_NoSort;
%csconstvalue("1024") ImGuiTableColumnFlags_NoSortAscending;
%csconstvalue("2048") ImGuiTableColumnFlags_NoSortDescending;
%csconstvalue("4096") ImGuiTableColumnFlags_NoHeaderLabel;
%csconstvalue("8192") ImGuiTableColumnFlags_NoHeaderWidth;
%csconstvalue("16384") ImGuiTableColumnFlags_PreferSortAscending;
%csconstvalue("32768") ImGuiTableColumnFlags_PreferSortDescending;
%csconstvalue("65536") ImGuiTableColumnFlags_IndentEnable;
%csconstvalue("131072") ImGuiTableColumnFlags_IndentDisable;
%csconstvalue("16777216") ImGuiTableColumnFlags_IsEnabled;
%csconstvalue("33554432") ImGuiTableColumnFlags_IsVisible;
%csconstvalue("67108864") ImGuiTableColumnFlags_IsSorted;
%csconstvalue("134217728") ImGuiTableColumnFlags_IsHovered;
%csconstvalue("24") ImGuiTableColumnFlags_WidthMask_;
%csconstvalue("196608") ImGuiTableColumnFlags_IndentMask_;
%csconstvalue("251658240") ImGuiTableColumnFlags_StatusMask_;
%csconstvalue("1073741824") ImGuiTableColumnFlags_NoDirectResize_;
%rename(ImGuiTableRowFlags) ImGuiTableRowFlags_;
%typemap(csattributes) ImGuiTableRowFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiTableRowFlags_None;
%csconstvalue("1") ImGuiTableRowFlags_Headers;
%csconstvalue("0") ImGuiTableBgTarget_None;
%csconstvalue("1") ImGuiTableBgTarget_RowBg0;
%csconstvalue("2") ImGuiTableBgTarget_RowBg1;
%csconstvalue("3") ImGuiTableBgTarget_CellBg;
%rename(ImGuiFocusedFlags) ImGuiFocusedFlags_;
%typemap(csattributes) ImGuiFocusedFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiFocusedFlags_None;
%csconstvalue("1") ImGuiFocusedFlags_ChildWindows;
%csconstvalue("2") ImGuiFocusedFlags_RootWindow;
%csconstvalue("4") ImGuiFocusedFlags_AnyWindow;
%csconstvalue("8") ImGuiFocusedFlags_NoPopupHierarchy;
%csconstvalue("16") ImGuiFocusedFlags_DockHierarchy;
%csconstvalue("3") ImGuiFocusedFlags_RootAndChildWindows;
%rename(ImGuiHoveredFlags) ImGuiHoveredFlags_;
%typemap(csattributes) ImGuiHoveredFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiHoveredFlags_None;
%csconstvalue("1") ImGuiHoveredFlags_ChildWindows;
%csconstvalue("2") ImGuiHoveredFlags_RootWindow;
%csconstvalue("4") ImGuiHoveredFlags_AnyWindow;
%csconstvalue("8") ImGuiHoveredFlags_NoPopupHierarchy;
%csconstvalue("16") ImGuiHoveredFlags_DockHierarchy;
%csconstvalue("32") ImGuiHoveredFlags_AllowWhenBlockedByPopup;
%csconstvalue("128") ImGuiHoveredFlags_AllowWhenBlockedByActiveItem;
%csconstvalue("256") ImGuiHoveredFlags_AllowWhenOverlapped;
%csconstvalue("512") ImGuiHoveredFlags_AllowWhenDisabled;
%csconstvalue("416") ImGuiHoveredFlags_RectOnly;
%csconstvalue("3") ImGuiHoveredFlags_RootAndChildWindows;
%rename(ImGuiDockNodeFlags) ImGuiDockNodeFlags_;
%typemap(csattributes) ImGuiDockNodeFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiDockNodeFlags_None;
%csconstvalue("1") ImGuiDockNodeFlags_KeepAliveOnly;
%csconstvalue("4") ImGuiDockNodeFlags_NoDockingInCentralNode;
%csconstvalue("8") ImGuiDockNodeFlags_PassthruCentralNode;
%csconstvalue("16") ImGuiDockNodeFlags_NoSplit;
%csconstvalue("32") ImGuiDockNodeFlags_NoResize;
%csconstvalue("64") ImGuiDockNodeFlags_AutoHideTabBar;
%rename(ImGuiDragDropFlags) ImGuiDragDropFlags_;
%typemap(csattributes) ImGuiDragDropFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiDragDropFlags_None;
%csconstvalue("1") ImGuiDragDropFlags_SourceNoPreviewTooltip;
%csconstvalue("2") ImGuiDragDropFlags_SourceNoDisableHover;
%csconstvalue("4") ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
%csconstvalue("8") ImGuiDragDropFlags_SourceAllowNullID;
%csconstvalue("16") ImGuiDragDropFlags_SourceExtern;
%csconstvalue("32") ImGuiDragDropFlags_SourceAutoExpirePayload;
%csconstvalue("1024") ImGuiDragDropFlags_AcceptBeforeDelivery;
%csconstvalue("2048") ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
%csconstvalue("4096") ImGuiDragDropFlags_AcceptNoPreviewTooltip;
%csconstvalue("3072") ImGuiDragDropFlags_AcceptPeekOnly;
%csconstvalue("-1") ImGuiDir_None;
%csconstvalue("0") ImGuiDir_Left;
%csconstvalue("1") ImGuiDir_Right;
%csconstvalue("2") ImGuiDir_Up;
%csconstvalue("3") ImGuiDir_Down;
%csconstvalue("0") ImGuiSortDirection_None;
%csconstvalue("1") ImGuiSortDirection_Ascending;
%csconstvalue("2") ImGuiSortDirection_Descending;
%rename(ImGuiKeyModFlags) ImGuiKeyModFlags_;
%typemap(csattributes) ImGuiKeyModFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiKeyModFlags_None;
%csconstvalue("1") ImGuiKeyModFlags_Ctrl;
%csconstvalue("2") ImGuiKeyModFlags_Shift;
%csconstvalue("4") ImGuiKeyModFlags_Alt;
%csconstvalue("8") ImGuiKeyModFlags_Super;
%csconstvalue("16") ImGuiNavInput_InternalStart_;
%rename(ImGuiConfigFlags) ImGuiConfigFlags_;
%typemap(csattributes) ImGuiConfigFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiConfigFlags_None;
%csconstvalue("1") ImGuiConfigFlags_NavEnableKeyboard;
%csconstvalue("2") ImGuiConfigFlags_NavEnableGamepad;
%csconstvalue("4") ImGuiConfigFlags_NavEnableSetMousePos;
%csconstvalue("8") ImGuiConfigFlags_NavNoCaptureKeyboard;
%csconstvalue("16") ImGuiConfigFlags_NoMouse;
%csconstvalue("32") ImGuiConfigFlags_NoMouseCursorChange;
%csconstvalue("64") ImGuiConfigFlags_DockingEnable;
%csconstvalue("1024") ImGuiConfigFlags_ViewportsEnable;
%csconstvalue("16384") ImGuiConfigFlags_DpiEnableScaleViewports;
%csconstvalue("32768") ImGuiConfigFlags_DpiEnableScaleFonts;
%csconstvalue("1048576") ImGuiConfigFlags_IsSRGB;
%csconstvalue("2097152") ImGuiConfigFlags_IsTouchScreen;
%rename(ImGuiBackendFlags) ImGuiBackendFlags_;
%typemap(csattributes) ImGuiBackendFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiBackendFlags_None;
%csconstvalue("1") ImGuiBackendFlags_HasGamepad;
%csconstvalue("2") ImGuiBackendFlags_HasMouseCursors;
%csconstvalue("4") ImGuiBackendFlags_HasSetMousePos;
%csconstvalue("8") ImGuiBackendFlags_RendererHasVtxOffset;
%csconstvalue("1024") ImGuiBackendFlags_PlatformHasViewports;
%csconstvalue("2048") ImGuiBackendFlags_HasMouseHoveredViewport;
%csconstvalue("4096") ImGuiBackendFlags_RendererHasViewports;
%rename(ImGuiButtonFlags) ImGuiButtonFlags_;
%typemap(csattributes) ImGuiButtonFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiButtonFlags_None;
%csconstvalue("1") ImGuiButtonFlags_MouseButtonLeft;
%csconstvalue("2") ImGuiButtonFlags_MouseButtonRight;
%csconstvalue("4") ImGuiButtonFlags_MouseButtonMiddle;
%csconstvalue("7") ImGuiButtonFlags_MouseButtonMask_;
%csconstvalue("1") ImGuiButtonFlags_MouseButtonDefault_;
%rename(ImGuiColorEditFlags) ImGuiColorEditFlags_;
%typemap(csattributes) ImGuiColorEditFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiColorEditFlags_None;
%csconstvalue("2") ImGuiColorEditFlags_NoAlpha;
%csconstvalue("4") ImGuiColorEditFlags_NoPicker;
%csconstvalue("8") ImGuiColorEditFlags_NoOptions;
%csconstvalue("16") ImGuiColorEditFlags_NoSmallPreview;
%csconstvalue("32") ImGuiColorEditFlags_NoInputs;
%csconstvalue("64") ImGuiColorEditFlags_NoTooltip;
%csconstvalue("128") ImGuiColorEditFlags_NoLabel;
%csconstvalue("256") ImGuiColorEditFlags_NoSidePreview;
%csconstvalue("512") ImGuiColorEditFlags_NoDragDrop;
%csconstvalue("1024") ImGuiColorEditFlags_NoBorder;
%csconstvalue("65536") ImGuiColorEditFlags_AlphaBar;
%csconstvalue("131072") ImGuiColorEditFlags_AlphaPreview;
%csconstvalue("262144") ImGuiColorEditFlags_AlphaPreviewHalf;
%csconstvalue("524288") ImGuiColorEditFlags_HDR;
%csconstvalue("1048576") ImGuiColorEditFlags_DisplayRGB;
%csconstvalue("2097152") ImGuiColorEditFlags_DisplayHSV;
%csconstvalue("4194304") ImGuiColorEditFlags_DisplayHex;
%csconstvalue("8388608") ImGuiColorEditFlags_Uint8;
%csconstvalue("16777216") ImGuiColorEditFlags_Float;
%csconstvalue("33554432") ImGuiColorEditFlags_PickerHueBar;
%csconstvalue("67108864") ImGuiColorEditFlags_PickerHueWheel;
%csconstvalue("134217728") ImGuiColorEditFlags_InputRGB;
%csconstvalue("268435456") ImGuiColorEditFlags_InputHSV;
%csconstvalue("177209344") ImGuiColorEditFlags_DefaultOptions_;
%csconstvalue("7340032") ImGuiColorEditFlags_DisplayMask_;
%csconstvalue("25165824") ImGuiColorEditFlags_DataTypeMask_;
%csconstvalue("100663296") ImGuiColorEditFlags_PickerMask_;
%csconstvalue("402653184") ImGuiColorEditFlags_InputMask_;
%csconstvalue("1048576") ImGuiColorEditFlags_RGB;
%csconstvalue("2097152") ImGuiColorEditFlags_HSV;
%csconstvalue("4194304") ImGuiColorEditFlags_HEX;
%rename(ImGuiSliderFlags) ImGuiSliderFlags_;
%typemap(csattributes) ImGuiSliderFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiSliderFlags_None;
%csconstvalue("16") ImGuiSliderFlags_AlwaysClamp;
%csconstvalue("32") ImGuiSliderFlags_Logarithmic;
%csconstvalue("64") ImGuiSliderFlags_NoRoundToFormat;
%csconstvalue("128") ImGuiSliderFlags_NoInput;
%csconstvalue("1879048207") ImGuiSliderFlags_InvalidMask_;
%csconstvalue("16") ImGuiSliderFlags_ClampOnInput;
%csconstvalue("0") ImGuiMouseButton_Left;
%csconstvalue("1") ImGuiMouseButton_Right;
%csconstvalue("2") ImGuiMouseButton_Middle;
%csconstvalue("5") ImGuiMouseButton_COUNT;
%csconstvalue("-1") ImGuiMouseCursor_None;
%csconstvalue("0") ImGuiMouseCursor_Arrow;
%csconstvalue("0") ImGuiCond_None;
%csconstvalue("1") ImGuiCond_Always;
%csconstvalue("2") ImGuiCond_Once;
%csconstvalue("4") ImGuiCond_FirstUseEver;
%csconstvalue("8") ImGuiCond_Appearing;
%rename(ImDrawFlags) ImDrawFlags_;
%typemap(csattributes) ImDrawFlags_ "[global::System.Flags]";
%csconstvalue("0") ImDrawFlags_None;
%csconstvalue("1") ImDrawFlags_Closed;
%csconstvalue("16") ImDrawFlags_RoundCornersTopLeft;
%csconstvalue("32") ImDrawFlags_RoundCornersTopRight;
%csconstvalue("64") ImDrawFlags_RoundCornersBottomLeft;
%csconstvalue("128") ImDrawFlags_RoundCornersBottomRight;
%csconstvalue("256") ImDrawFlags_RoundCornersNone;
%csconstvalue("48") ImDrawFlags_RoundCornersTop;
%csconstvalue("192") ImDrawFlags_RoundCornersBottom;
%csconstvalue("80") ImDrawFlags_RoundCornersLeft;
%csconstvalue("160") ImDrawFlags_RoundCornersRight;
%csconstvalue("240") ImDrawFlags_RoundCornersAll;
%csconstvalue("240") ImDrawFlags_RoundCornersDefault_;
%csconstvalue("496") ImDrawFlags_RoundCornersMask_;
%rename(ImDrawListFlags) ImDrawListFlags_;
%typemap(csattributes) ImDrawListFlags_ "[global::System.Flags]";
%csconstvalue("0") ImDrawListFlags_None;
%csconstvalue("1") ImDrawListFlags_AntiAliasedLines;
%csconstvalue("2") ImDrawListFlags_AntiAliasedLinesUseTex;
%csconstvalue("4") ImDrawListFlags_AntiAliasedFill;
%csconstvalue("8") ImDrawListFlags_AllowVtxOffset;
%rename(ImFontAtlasFlags) ImFontAtlasFlags_;
%typemap(csattributes) ImFontAtlasFlags_ "[global::System.Flags]";
%csconstvalue("0") ImFontAtlasFlags_None;
%csconstvalue("1") ImFontAtlasFlags_NoPowerOfTwoHeight;
%csconstvalue("2") ImFontAtlasFlags_NoMouseCursors;
%csconstvalue("4") ImFontAtlasFlags_NoBakedLines;
%rename(ImGuiViewportFlags) ImGuiViewportFlags_;
%typemap(csattributes) ImGuiViewportFlags_ "[global::System.Flags]";
%csconstvalue("0") ImGuiViewportFlags_None;
%csconstvalue("1") ImGuiViewportFlags_IsPlatformWindow;
%csconstvalue("2") ImGuiViewportFlags_IsPlatformMonitor;
%csconstvalue("4") ImGuiViewportFlags_OwnedByApp;
%csconstvalue("8") ImGuiViewportFlags_NoDecoration;
%csconstvalue("16") ImGuiViewportFlags_NoTaskBarIcon;
%csconstvalue("32") ImGuiViewportFlags_NoFocusOnAppearing;
%csconstvalue("64") ImGuiViewportFlags_NoFocusOnClick;
%csconstvalue("128") ImGuiViewportFlags_NoInputs;
%csconstvalue("256") ImGuiViewportFlags_NoRendererClear;
%csconstvalue("512") ImGuiViewportFlags_TopMost;
%csconstvalue("1024") ImGuiViewportFlags_Minimized;
%csconstvalue("2048") ImGuiViewportFlags_NoAutoMerge;
%csconstvalue("4096") ImGuiViewportFlags_CanHostOtherWindows;
%rename(ImDrawCornerFlags) ImDrawCornerFlags_;
%typemap(csattributes) ImDrawCornerFlags_ "[global::System.Flags]";
%csconstvalue("256") ImDrawCornerFlags_None;
%csconstvalue("16") ImDrawCornerFlags_TopLeft;
%csconstvalue("32") ImDrawCornerFlags_TopRight;
%csconstvalue("64") ImDrawCornerFlags_BotLeft;
%csconstvalue("128") ImDrawCornerFlags_BotRight;
%csconstvalue("240") ImDrawCornerFlags_All;
%csconstvalue("48") ImDrawCornerFlags_Top;
%csconstvalue("192") ImDrawCornerFlags_Bot;
%csconstvalue("80") ImDrawCornerFlags_Left;
%csconstvalue("160") ImDrawCornerFlags_Right;
%csattribute(ImGuiTextFilter, %arg(bool), IsActive, IsActive);
%csattribute(ImDrawCmd, %arg(ImTextureID), TexID, GetTexID);
%csattribute(ImDrawList, %arg(ImVec2), ClipRectMin, GetClipRectMin);
%csattribute(ImDrawList, %arg(ImVec2), ClipRectMax, GetClipRectMax);
%csattribute(ImFontAtlasCustomRect, %arg(bool), IsPacked, IsPacked);
%csattribute(ImFontAtlas, %arg(bool), IsBuilt, IsBuilt);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesDefault, GetGlyphRangesDefault);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesKorean, GetGlyphRangesKorean);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesJapanese, GetGlyphRangesJapanese);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesChineseFull, GetGlyphRangesChineseFull);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesChineseSimplifiedCommon, GetGlyphRangesChineseSimplifiedCommon);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesCyrillic, GetGlyphRangesCyrillic);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesThai, GetGlyphRangesThai);
%csattribute(ImFontAtlas, %arg(ImWchar *), GlyphRangesVietnamese, GetGlyphRangesVietnamese);
%csattribute(ImFont, %arg(bool), IsLoaded, IsLoaded);
%csattribute(ImFont, %arg(char *), DebugName, GetDebugName);
%csattribute(ImGuiViewport, %arg(ImVec2), Center, GetCenter);
%csattribute(ImGuiViewport, %arg(ImVec2), WorkCenter, GetWorkCenter);
