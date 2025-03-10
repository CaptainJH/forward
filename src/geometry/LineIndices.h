//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) Jason Zink 
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// LineIndices 
//
// This class represents the two indices of a line.
//--------------------------------------------------------------------------------
#ifndef LineIndices_h
#define LineIndices_h
#include "types.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class LineIndices
	{
	public:
		LineIndices();
		LineIndices( u32 P1, u32 P2 );
		~LineIndices();

		LineIndices& operator= ( const LineIndices& Line );

		// member access
		u32 P1( ) const;
		u32& P1( );
		u32 P2( ) const;
		u32& P2( );

		void swapP1P2( );

	protected:
		u32 m_uiIndices[2];
	};
};
//--------------------------------------------------------------------------------
#endif // LineIndices_h
