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
// PointIndices 
//
// This class represents the single index of a point.
//--------------------------------------------------------------------------------
#ifndef PointIndices_h
#define PointIndices_h
#include "types.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class PointIndices
	{
	public:
		PointIndices();
		PointIndices( u32 P1 );
		~PointIndices();

		PointIndices& operator= ( const PointIndices& Point );

		// member access
		u32 P1( ) const;
		u32& P1( );

	protected:
		u32 m_uiIndices[1];
	};
};
//--------------------------------------------------------------------------------
#endif // PointIndices_h
