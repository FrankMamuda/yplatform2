/*
===========================================================================
Copyright (C) 2011-2012 Edd 'Double Dee' Psycho

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

#ifndef UI_ITEM_H
#define UI_ITEM_H

//
// includes
//
#include "ui_main.h"
#include <QtScript>

//
// class:UiItem
//
class UiItem : public QObject, protected QScriptable {
    Q_OBJECT

    // global
    Q_PROPERTY( QString name READ name WRITE setName SCRIPTABLE true )
    Q_PROPERTY( bool visible READ isVisible WRITE setVisible SCRIPTABLE true )
    Q_PROPERTY( Types type READ type WRITE setType )
    Q_PROPERTY( QPointF pos READ pos WRITE setPos )
    Q_PROPERTY( QColor colour READ colour WRITE setColour )
    Q_PROPERTY( UiItem *parent READ parent WRITE setParent )
    Q_PROPERTY( UiMain::Context context READ context WRITE setContext )
    Q_ENUMS( Types )
    Q_ENUMS( Alignment )

    // window specific
    Q_PROPERTY( QColor backColour READ backColour WRITE setBackColour )
    Q_PROPERTY( QSizeF size READ size WRITE setSize )
    Q_PROPERTY( QString material READ material WRITE setMaterial )
    Q_PROPERTY( bool mouseFocus READ hasMouseFocus WRITE setMouseFocus RESET clearMouseFocus )
    Q_PROPERTY( bool x READ x )
    Q_PROPERTY( bool y READ y )
    Q_PROPERTY( bool width READ width )
    Q_PROPERTY( bool height READ height )
    //Q_PROPERTY( bool focus READ hasFocus WRITE setFocus RESET clearFocus )

    // text specific
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QFont font READ font WRITE setFont )
    Q_PROPERTY( QString fontFamily READ fontFamily WRITE setFontFamily )
    Q_PROPERTY( int pointSize READ pointSize WRITE setPointSize )
    Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    Q_ENUMS( Alignment )

public:
    enum Types {
        Container = 0,
        Window,
        Text
    };
    enum Alignment {
        LeftAligned = 0,
        Centred
    };

    // global
    explicit UiItem( Types type = Container, const QString &string = QString());
    QString name() const { return this->m_name; }
    bool isVisible() const { return this->m_visibility; }
    Types type() const { return this->m_type; }
    QPointF pos() const { return this->m_pos; }
    int x() const { return this->pos().x(); }
    int y() const { return this->pos().y(); }
    QColor colour() const { return m_colour; }
    UiItem *parent() const { return m_parent; }
    bool hasParent() const { if ( this->parent() == NULL ) return false; else return true; }
    UiMain::Context context() const { return this->m_context; }

    // window specific
    QColor backColour() const { return m_backColour; }
    QSizeF size() const { return m_size; }
    int width() const { return this->size().width(); }
    int height() const { return this->size().height(); }
    QRect rect() const { return QRect( this->x(), this->y(), this->width(), this->height()); }
    bool contains( QPoint position ) { if ( this->rect().contains( position )) return true; return false; }
    bool contains( int x, int y ) { return this->contains( QPoint( x, y )); }
    QString material() const { return m_material; }
    bool hasMouseFocus() const { return m_mouseFocus; }
    //bool hasFocus() const { return m_focus; }

    // text specific
    QString text() const { return this->m_text; }
    QFont font() const { return this->m_font; }
    QString fontFamily() const { return this->m_fontFamily; }
    int pointSize() const { return this->m_font.pixelSize(); }
    Alignment alignment() const { return this->m_alignment; }

    // general functions
    void render( UiMain::Context parentContext );
    void checkMouseEvent( ModuleAPI::KeyEventType type, int key );
    void checkMouseOver();
    void checkKeyEvent( ModuleAPI::KeyEventType type, int key );

signals:
    void mouseOver( int x, int y, QObject* );
    void mouseExit( int x, int y, QObject* );
    void mouseEvent( int x, int y, int type, int key, QObject* );
    void keyEvent( int key, int type, QObject* );
    //void mouseLeftClick();
    //void mouseRightClick();

protected:
    // global
    QString m_name;
    bool m_visibility;
    Types m_type;
    QPointF m_pos;
    QColor m_colour;
    UiItem *m_parent;
    UiMain::Context m_context;

    // window specific
    QColor m_backColour;
    QSizeF m_size;
    QString m_material;
    bool m_mouseFocus;
    //bool m_focus;

    // text specific
    QString m_text;
    QFont m_font;
    QString m_fontFamily;
    Alignment m_alignment;

    // childern container
    QList<UiItem*>children;

private slots:

public slots:
    // global
    void setName( const QString &name ) { this->m_name = name; }
    void setVisible( bool visibility = true ) { this->m_visibility = visibility; }
    void hide() { this->m_visibility = false; }
    void show() { this->m_visibility = true; }
    void setType( Types type = Container ) { this->m_type = type; }
    void addChild( UiItem *item ) { this->children << item; }
    void setPos( const QPointF &pos ) { this->m_pos = pos; }
    void setPos( float x, float y ) { this->m_pos = QPointF( x, y ); }
    void setColour( const QColor &colour ) { this->m_colour = colour; }
    void setColour( float r, float g, float b, float a = 1.0f ) { this->m_colour = QColor::fromRgbF( r, g, b, a ); }
    void setColour( UI::Vec4D colour ) { this->m_colour = QColor::fromRgbF( colour[0], colour[1], colour[2], colour[3] ); }
    void setParent( UiItem *parentPtr );
    void setContext( UiMain::Context context ) { this->m_context = context; }
    void setContext( int context ) { this->m_context = static_cast<UiMain::Context>( context ); }

    // window specific
    void setSize( const QSizeF &size ) { this->m_size = size; }
    void setSize( float w, float h ) { this->m_size = QSizeF( w, h ); }
    void setRect( const QRect &rect ) { this->m_pos = QPointF( rect.x(), rect.y()), this->m_size = QSizeF( rect.width(), rect.height()); }
    void setRect( float x, float y, float w, float h ) { this->m_pos = QPointF( x, y ), this->m_size = QSizeF( w, h ); }
    void setBackColour( const QColor &colour ) { this->m_backColour = colour; }
    void setBackColour( float r, float g, float b, float a = 1.0f ) { this->m_backColour = QColor::fromRgbF( r, g, b, a ); }
    void setBackColour( UI::Vec4D colour ) { this->m_backColour = QColor::fromRgbF( colour[0], colour[1], colour[2], colour[3] ); }
    void setMaterial( const QString &mtr ) { this->m_material = mtr; }
    void setMouseFocus( bool focus = true ) { this->m_mouseFocus = focus; }
    void clearMouseFocus() { this->m_mouseFocus = false; }
    //void setFocus( bool focus = true ) { this->m_focus = focus; }
    //void clearFocus() { this->m_focus = false; }

    // text specific
    void setText( const QString &text ) { this->m_text = text; }
    void setFont( const QFont &font ) { this->m_font = font; }
    void setFontFamily( const QString &fontFamily ) { this->m_fontFamily = fontFamily; this->m_font = QFont( fontFamily ); }
    void setPointSize( int size ) { this->m_font.setPixelSize( size ); }
    void setAlignment( Alignment alignment ) { this->m_alignment = alignment; }
    void setCentred() { this->m_alignment = Centred; }
    void setLeftAligned() { this->m_alignment = LeftAligned; }
};

Q_DECLARE_METATYPE( UiItem* )

#endif // UI_ITEM_H
