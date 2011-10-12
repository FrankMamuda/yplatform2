/*
===========================================================================
Copyright (C) 2011 Edd 'Double Dee' Psycho

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
// material library: commons
//

var platform = new material( ":/common/platform" );
stage( ":/common/platform", platform );

var whiteimage = new material( ":/common/white" );
stage( ":/common/white", whiteimage );

// ui stuff, delete me
var frame_ur = new material( ":/ui/frame_corner_ur" )
print( "##### create awesome mtr\n")
stage( ":/assets/frame_corner", frame_ur );

var frame_ul = new material( ":/ui/frame_corner_ul" )
var frame_ul_s0 = new stage( ":/assets/frame_corner", frame_ul );
frame_ul_s0.setTextureMod( "transform", 1, -1, 0, 0 );

var frame_ll = new material( ":/ui/frame_corner_ll" )
var frame_ll_s0 = new stage( ":/assets/frame_corner", frame_ll );
frame_ll_s0.setTextureMod( "transform", 1, 1, 0, 0 );

var frame_lr = new material( ":/ui/frame_corner_lr" )
var frame_lr_s0 = new stage( ":/assets/frame_corner", frame_lr );
frame_lr_s0.setTextureMod( "transform", -1, 1, 0, 0 );
