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
// TriangleIndices 
//
// This class represents the three indices of a triangle.
//--------------------------------------------------------------------------------
#ifndef TriangleIndices_h
#define TriangleIndices_h
#include "Types.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class TriangleIndices
	{
	public:
		TriangleIndices();
		TriangleIndices( u32 P1, u32 P2, u32 P3 );
		~TriangleIndices();

		TriangleIndices& operator= ( const TriangleIndices& Triangle );

		// member access
		u32 P1( ) const;
		u32& P1( );
		u32 P2( ) const;
		u32& P2( );
		u32 P3( ) const;
		u32& P3( );

		void swapP1P2( );
		void swapP2P3( );
		void swapP3P1( );

	protected:
		u32 m_uiIndices[3];
	};
};
//--------------------------------------------------------------------------------
#endif // TriangleIndices_h
