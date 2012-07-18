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

//
// includes
//
#include "../mod_trap.h"
#include "ui_item.h"
#include "ui_main.h"
#include "../../common/sys_common.h"

/*
================
construct
================
*/
UiItem::UiItem( Types type, const QString &string ) {
    this->setName( string );
    this->setType( type );
    this->setPos( this->pos());// + qobject_cast<UiItem*>( parent )->pos());
    this->setVisible( true );
    this->m_parent = NULL;
    this->clearMouseFocus();
    this->setContext( UiMain::Default );

    // add to list regardless if parentless or not
    m.itemList << this;
}

/*
================
setParent
================
*/
void UiItem::setParent( UiItem *parentPtr ) {
    if ( parentPtr == this ) {
        com.print( StrWarn + this->tr( "can't set self\n" ));
        return;
    }

    if ( !this->hasParent()) {
        this->m_parent = parentPtr;
        parentPtr->addChild( this );
    } else {
        if ( this->name().isEmpty())
            com.print( StrWarn + this->tr( "parent has been already set\n" ));
        else
            com.print( StrWarn + this->tr( "parent has been already set for item \"%1\"\n" ).arg( this->name()));
    }
}

/*
================
render
================
*/
void UiItem::render( UiMain::Context parentContext ) {
    // abort if invisible
    if ( !this->isVisible())
        return;

    // return if different context
    if ( parentContext != m.currentContext())
        return;

    // handle types
    switch ( this->type()) {
    // render window
    case Window:
        // draw background
        if ( !this->size().isEmpty() && this->backColour().isValid()) {
            r.setColour( this->backColour());

            // draw material
            r.drawMaterial( this->pos().x(), this->pos().y(), this->size().width(), this->size().height(), r.loadMaterial( ":/common/white" ));

            // reset colour
            r.setColour();
        }

        // draw material
        if ( !this->material().isEmpty() && !this->size().isEmpty()) {
            if ( this->colour().isValid())
                r.setColour( this->colour());

            // draw material
            r.drawMaterial( this->pos().x(), this->pos().y(), this->size().width(), this->size().height(), r.loadMaterial( this->material()));

            // reset colour
            r.setColour();
        }
        break;

        // render text
    case Text:
        // draw itself
        if ( !this->text().isEmpty()) {
            if ( this->alignment() == Centred ) {
                QFontMetrics fm( this->font());
                float tw = fm.width( this->text());

                // origin is the centre
                r.drawText( this->m_pos.x() - tw / 2.0f, this->m_pos.y(), this->m_font, this->text(), this->colour());
            } else
                r.drawText( this->m_pos.x(), this->m_pos.y(), this->m_font, this->text(), this->colour());
        }
        break;

    default:
    case Container:
        break;
    }

    // render children last
    foreach ( UiItem *childPtr, this->children )
        childPtr->render( parentContext );
}

/*
================
checkMouseEvent
================
*/
void UiItem::checkMouseEvent( ModuleAPI::KeyEventType type, int key ) {
    // abort if invisible
    if ( !this->isVisible())
        return;

    // check mouse events
    if ( this->contains( m.mousePos ))
        emit this->mouseEvent( m.mousePos.x(), m.mousePos.y(), ( int )type, key, this );

    // just pass it on
    foreach ( UiItem *childPtr, this->children )
        childPtr->checkMouseEvent( type, key );
}


/*
================
checkMouseOver
================
*/
void UiItem::checkMouseOver() {
    // abort if invisible
    if ( !this->isVisible())
        return;

    // check mouse events
    if ( this->contains( m.mousePos )) {
        if ( !this->hasMouseFocus()) {
            this->setMouseFocus();
            emit this->mouseOver( m.mousePos.x(), m.mousePos.y(), this );
        }
    } else {
        if ( this->hasMouseFocus()) {
            this->clearMouseFocus();
            emit this->mouseExit( m.mousePos.x(), m.mousePos.y(), this );
        }
    }

    // just pass it on
    foreach ( UiItem *childPtr, this->children )
        childPtr->checkMouseOver();
}

/*
================
checkKeyEvent

 pass events to the focused item? (and it's children)
================
*/
void UiItem::checkKeyEvent( ModuleAPI::KeyEventType type, int key ) {
    // abort if invisible
    if ( !this->isVisible())
        return;

    // handle key events
    emit this->keyEvent( key, type, this );
    /* some code */
    //com.print( QString( "key %1\n" ).arg( key ));

    // just pass it on
    foreach ( UiItem *childPtr, this->children )
        childPtr->checkKeyEvent( type, key );
}
