#ifndef STYLE_H
#define STYLE_H

#include <QProxyStyle>
#include <QStyleOptionTab>
#include <QTextOption>

#include "global.h"
#include "blurhelper.h"

class PROXYSTYLESHARED_EXPORT Style : public QProxyStyle
{
    Q_OBJECT

public:
    //* metrics
    enum Metrics {
        // frames
        Frame_FrameWidth = 4,
        Frame_FrameRadius = 5,

        // layout
        Layout_TopLevelMarginWidth = 10,
        Layout_ChildMarginWidth = 6,
        Layout_DefaultSpacing = 6,

        // line editors
        LineEdit_FrameWidth = 3,
        LineEdit_MarginHeight = 2,
        LineEdit_MarginWidth = 8,
        LineEdit_MinHeight = 36,
        LineEdit_MinWidth = 80,

        // menu items
        Menu_FrameWidth = 0,
        MenuItem_MarginWidth = 4,
        MenuItem_ItemSpacing = 6,
        MenuItem_AcceleratorSpace = 16,
        MenuButton_IndicatorWidth = 20,

        // combobox
        ComboBox_FrameWidth = 4,
        ComboBox_MarginHeight = 4,
        ComboBox_MarginWidth = 8,
        ComboBox_MinHeight = 36,
        ComboBox_MinWidth = 80,

        // spinbox
        SpinBox_FrameWidth = LineEdit_FrameWidth,
        SpinBox_ArrowButtonWidth = 20,
        SpinBox_MinHeight = 36,
        SpinBox_MinWidth = 80,
        SpinBox_MarginHeight = 4,
        SpinBox_MarginWidth = 8,

        // groupbox title margin
        GroupBox_TitleMarginWidth = 4,

        // buttons
        Button_MinHeight = 36,
        Button_MinWidth = 80,
        Button_MarginHeight = 4,
        Button_MarginWidth = 8,
        Button_ItemSpacing = 4,

        // tool buttons
        ToolButton_MarginWidth = 6,
        ToolButton_ItemSpacing = 4,
        ToolButton_InlineIndicatorWidth = 12,

        // checkboxes and radio buttons
        CheckBox_Size = 22,
        CheckBox_FocusMarginWidth = 3,
        CheckBox_ItemSpacing = 8,

        // menubar items
        MenuBarItem_MarginWidth = 8,
        MenuBarItem_MarginHeight = 5,

        // scrollbars
        ScrollBar_Extend = 14,
        ScrollBar_SliderWidth = 10,
        ScrollBar_MinSliderHeight = 24,
        ScrollBar_NoButtonHeight = (ScrollBar_Extend - ScrollBar_SliderWidth) / 2,
        ScrollBar_SingleButtonHeight = 0,
        ScrollBar_DoubleButtonHeight = 0,

        // toolbars
        ToolBar_FrameWidth = 2,
        ToolBar_HandleExtent = 10,
        ToolBar_HandleWidth = 6,
        ToolBar_SeparatorWidth = 8,
        ToolBar_ExtensionWidth = 20,
        ToolBar_ItemSpacing = 0,

        // progressbars
        ProgressBar_BusyIndicatorSize = 24,
        ProgressBar_Thickness = 3,
        ProgressBar_ItemSpacing = 3,

        // mdi title bar
        TitleBar_MarginWidth = 4,

        // sliders
        Slider_TickLength = 4,
        Slider_TickMarginWidth = 6,
        Slider_GrooveThickness = 3,
        Slider_ControlThickness = 24,

        // tabbar
        TabBar_TabMarginHeight = 9,
        TabBar_TabMarginWidth = 8,
        TabBar_TabMinWidth = 80,
        TabBar_TabMinHeight = 36,
        TabBar_TabItemSpacing = 8,
        TabBar_TabOverlap = 1,
        TabBar_BaseOverlap = 0,

        // tab widget
        TabWidget_MarginWidth = 4,

        // toolbox
        ToolBox_TabMinWidth = 80,
        ToolBox_TabItemSpacing = 4,
        ToolBox_TabMarginWidth = 8,

        // tooltips
        ToolTip_FrameWidth = 3,

        // scroll areas
        ScrollArea_FrameWidth = 2,

        // list headers
        Header_MarginWidth = 3,
        Header_ItemSpacing = 2,
        Header_ArrowSize = 10,

        // tree view
        ItemView_ArrowSize = 10,
        ItemView_ItemMarginWidth = 3,
        SidePanel_ItemMarginWidth = 4,

        // splitter
        Splitter_SplitterWidth = 1,

        // shadow dimensions
        Shadow_Overlap = 0

    };

public:
    explicit Style();

    void polish(QWidget *w);
    void unpolish(QWidget *w);
    void polish(QApplication *app);

    void polish(QPalette &palette);

    void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    QRect subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *opt, QStyle::SubControl sc, const QWidget *w) const;

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                      QPainter *painter, const QWidget *widget) const;

    int styleHint(QStyle::StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *shret) const;
    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;

private:
    QRect centerRect(const QRect &rect, int width, int height) const {
        return QRect(rect.left() + (rect.width() - width) / 2, rect.top() + (rect.height() - height) / 2, width, height);
    }

    QRect visualRect(const QStyleOption *opt, const QRect &subRect) const {
        return QProxyStyle::visualRect(opt->direction, opt->rect, subRect);
    }

    // helper functions...

    bool drawTabBar(QPainter *painter, const QStyleOptionTab *tab, const QWidget *widget) const;
    bool drawTabBarLabel(QPainter *painter, const QStyleOptionTab *tab, const QWidget *widget) const;
    void drawMenu(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

    QRect scrollBarInternalSubControlRect(const QStyleOptionComplex *option, SubControl subControl) const;
    QRect scrollBarSubControlRect(const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const;

    void drawPanelButtonCommandPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
    void drawPanelButtonToolPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;

    // utils
    QString calculateElidedText(const QString &text, const QTextOption &textOption,
                                const QFont &font, const QRect &textRect, const Qt::Alignment valign,
                                Qt::TextElideMode textElideMode, int flags,
                                bool lastVisibleLineShouldBeElided, QPointF *paintStartPosition) const;
    void viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const;
    QString toolButtonElideText(const QStyleOptionToolButton *toolbutton,
                                const QRect &textRect, int flags) const;

private:
    BlurHelper *m_blurHelper;
};

#endif
