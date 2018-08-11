//***************************************************************************************
// FrameGraphObject.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <assert.h>
#include "SmartPtrs.h"
#include "render/ResourceSystem/DeviceObject.h"

namespace forward
{
	class DeviceObject;

	enum FrameGraphObjectType
	{
		FGOT_GRAPHICS_OBJECT,  // abstract
			FGOT_RESOURCE,  // abstract
				FGOT_BUFFER,  // abstract
					FGOT_CONSTANT_BUFFER,
					FGOT_VERTEX_BUFFER,
					FGOT_INDEX_BUFFER,
				FGOT_TEXTURE,  // abstract
					FGOT_TEXTURE1,
					FGOT_TEXTURE2,
					FGOT_TEXTURECUBE,
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

	class FrameGraphObject : public intrusive_ref_counter
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

		void SetDeviceObject(forward::DeviceObject* obj);
		void SetDeviceObject(DeviceObjPtr p);
		inline DeviceObjPtr DeviceObject();

		template<class T = FrameGraphObject>
		static shared_ptr<T> FindFrameGraphObject(const std::string& name)
		{
			for (auto ptr : m_sFGObjs)
			{
				if (!ptr.expired())
				{
					auto shared = ptr.lock_down<T>();
					if (shared && shared->Name() == name)
					{
						return shared;
					}
				}
			}

			return nullptr;
		}
		static void RegisterObject(FrameGraphObject* ptr);
		static void CheckMemoryLeak();

	protected:
		FrameGraphObjectType	m_type;
		std::string				m_name;

		DeviceObjPtr m_deviceObjectPtr = nullptr;

		static std::vector<weak_ptr<FrameGraphObject>>	m_sFGObjs;

	private:
		virtual void PostSetDeviceObject() {}
	};

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

	DeviceObjPtr FrameGraphObject::DeviceObject()
	{
		return m_deviceObjectPtr;
	}

	template<class T>
	T device_cast(FrameGraphObject* obj)
	{
		assert(obj);
		return static_cast<T>(obj->DeviceObject().get());
	}

	template<class T>
	T device_cast(forward::shared_ptr<forward::FrameGraphObject> p)
	{
		assert(p);
		return static_cast<T>(p->DeviceObject().get());
	}

}