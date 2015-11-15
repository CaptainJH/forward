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
// TStateArrayMonitor
//
//--------------------------------------------------------------------------------
#ifndef TStateArrayMonitor_h
#define TStateArrayMonitor_h
//--------------------------------------------------------------------------------
#include <assert.h>
#include <Types.h>
//--------------------------------------------------------------------------------
namespace forward
{
	template <class T, u32 N>
	class TStateArrayMonitor
	{
	public:
		TStateArrayMonitor( T initialState );
		~TStateArrayMonitor();

		void SetSister( TStateArrayMonitor<T,N>* pSister );
		bool SameAsSister( u32 slot );
		
		void SetState( u32 slot, T state );
	
		bool IsUpdateNeeded();
		u32 GetStartSlot();
		u32 GetEndSlot();
		u32 GetRange();

		T GetState( u32 slot ) const;
		T* GetFirstSlotLocation();
		T* GetSlotLocation( u32 slot );

		void InitializeStates();
		void ResetTracking();

	private:

		void SearchFromBelow();
		void SearchFromAbove();

		// The monitoring varaibles
		u32 m_uiStartSlot;
		u32 m_uiEndSlot;
		bool m_bUploadNeeded;

		// The state data
		T m_InitialState;
		T m_States[N];

		// The sister state
		TStateArrayMonitor<T,N>* m_pSister;
	};

#include "TStateArrayMonitor.inl"
};
//--------------------------------------------------------------------------------
#endif // TStateArrayMonitor_h
