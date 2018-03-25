//***************************************************************************************
// FrameGraphObject.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <assert.h>
#include "render/ResourceSystem/DeviceObject.h"

namespace forward
{
	enum FrameGraphObjectType
	{
		FGOT_GRAPHICS_OBJECT,  // abstract
			FGOT_RESOURCE,  // abstract
				FGOT_BUFFER,  // abstract
					FGOT_CONSTANT_BUFFER,
					FGOT_VERTEX_BUFFER,
					FGOT_INDEX_BUFFER,
				FGOT_TEXTURE,  // abstract
					FGOT_TEXTURE2,
			FGOT_SHADER,  // abstract
				FGOT_VERTEX_SHADER,
				FGOT_GEOMETRY_SHADER,
				FGOT_PIXEL_SHADER,
				FGOT_COMPUTE_SHADER,
			FGOT_DRAWING_STATE,  // abstract
				FGOT_SAMPLER_STATE,
				FGOT_BLEND_STATE,
				FGOT_DEPTH_STENCIL_STATE,
				FGOT_RASTERIZER_STATE,
			FGOT_VERTEX_INPUT_LAYOUT,
		FGOT_NUM_TYPES
	};

	class FrameGraphObject
	{
	public:
		FrameGraphObject();
		virtual ~FrameGraphObject();

		const std::string& Name() const;
		void SetName(const std::string& name);

		// Run-time type information.
		inline FrameGraphObjectType GetType() const;
		inline bool IsBuffer() const;
		inline bool IsTexture() const;
		inline bool IsShader() const;
		inline bool IsDrawingState() const;

		inline void SetDeviceObject(forward::DeviceObject* obj);
		inline forward::DeviceObject* DeviceObject();

		static FrameGraphObject* FindFrameGraphObject(const std::string& name);
		static void RegisterObject(FrameGraphObject* ptr);

	protected:
		FrameGraphObjectType	m_type;
		std::string				m_name;

		forward::DeviceObject*	m_deviceObjectPtr = nullptr;

		static std::vector<FrameGraphObject*>	m_sFGObjs;

	private:
		virtual void PostSetDeviceObject() {}
	};

	typedef std::shared_ptr<FrameGraphObject> ObjectPtr;

	FrameGraphObjectType FrameGraphObject::GetType() const
	{
		return m_type;
	}

	bool FrameGraphObject::IsBuffer() const
	{
		return m_type > FGOT_BUFFER && m_type < FGOT_TEXTURE;
	}

	bool FrameGraphObject::IsTexture() const
	{
		return m_type > FGOT_TEXTURE && m_type < FGOT_SHADER;
	}

	bool FrameGraphObject::IsShader() const
	{
		return m_type > FGOT_SHADER && m_type < FGOT_DRAWING_STATE;
	}

	bool FrameGraphObject::IsDrawingState() const
	{
		return m_type > FGOT_DRAWING_STATE && m_type < FGOT_NUM_TYPES;
	}

	void FrameGraphObject::SetDeviceObject(forward::DeviceObject* obj)
	{
		m_deviceObjectPtr = obj;
		PostSetDeviceObject();
	}

	DeviceObject* FrameGraphObject::DeviceObject()
	{
		return m_deviceObjectPtr;
	}

	template<class T>
	T device_cast(FrameGraphObject* obj)
	{
		assert(obj);
		return static_cast<T>(obj->DeviceObject());
	}
}