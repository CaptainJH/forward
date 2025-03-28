//***************************************************************************************
// GraphicsObject.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <assert.h>
#include "SmartPtrs.h"
#include "RHI/ResourceSystem/DeviceObject.h"

namespace forward
{
	class DeviceObject;

	enum GraphicsObjectType
	{
		FGOT_GRAPHICS_OBJECT,  // abstract
			FGOT_RESOURCE,  // abstract
				FGOT_BUFFER,  // abstract
					FGOT_CONSTANT_BUFFER,
					FGOT_VERTEX_BUFFER,
					FGOT_INDEX_BUFFER,
					FGOT_STRUCTURED_BUFFER,
					FGOT_SHADER_TABLE,
				FGOT_TEXTURE,  // abstract
					FGOT_TEXTURE1,
					FGOT_TEXTURE2,
					FGOT_TEXTURECUBE,
			FGOT_SHADER,  // abstract
				FGOT_VERTEX_SHADER,
				FGOT_GEOMETRY_SHADER,
				FGOT_PIXEL_SHADER,
				FGOT_COMPUTE_SHADER,
				FGOT_RT_SHADER,
			FGOT_PIPELINE_STATE,  // abstract
				FGOT_RASTER_PSO,
				FGOT_RT_PSO,
				FGOT_COMPUTE_PSO,
				FGOT_SAMPLER_STATE,
				FGOT_BLEND_STATE,
				FGOT_DEPTH_STENCIL_STATE,
				FGOT_RASTERIZER_STATE,
			FGOT_VERTEX_INPUT_LAYOUT,
		FGOT_NUM_TYPES
	};

	class GraphicsObject : public intrusive_ref_counter
	{
	public:
		GraphicsObject();
		virtual ~GraphicsObject();

		const std::string& Name() const;
		void SetName(const std::string& name);

		// Run-time type information.
		inline GraphicsObjectType GetType() const;
		inline bool IsBuffer() const;
		inline bool IsTexture() const;
		inline bool IsShader() const;
		inline bool IsDrawingState() const;

		void SetDeviceObject(DeviceObjPtr p);
		inline DeviceObjPtr DeviceObject();

		template<class T = GraphicsObject>
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
		static void RegisterObject(GraphicsObject* ptr);
		static void CheckMemoryLeak();

	protected:
		GraphicsObjectType 	m_type;
		std::string				m_name;

		DeviceObjPtr m_deviceObjectPtr = nullptr;

		static std::vector<weak_ptr<GraphicsObject>>	m_sFGObjs;
	};

	GraphicsObjectType GraphicsObject::GetType() const
	{
		return m_type;
	}

	bool GraphicsObject::IsBuffer() const
	{
		return m_type > FGOT_BUFFER && m_type < FGOT_TEXTURE;
	}

	bool GraphicsObject::IsTexture() const
	{
		return m_type > FGOT_TEXTURE && m_type < FGOT_SHADER;
	}

	bool GraphicsObject::IsShader() const
	{
		return m_type > FGOT_SHADER && m_type < FGOT_PIPELINE_STATE;
	}

	bool GraphicsObject::IsDrawingState() const
	{
		return m_type > FGOT_PIPELINE_STATE && m_type < FGOT_NUM_TYPES;
	}

	DeviceObjPtr GraphicsObject::DeviceObject()
	{
		return m_deviceObjectPtr;
	}

	template<class T>
	T device_cast(GraphicsObject* obj)
	{
		assert(obj);
		return static_cast<T>(obj->DeviceObject().get());
	}

	template<class T>
	T device_cast(forward::shared_ptr<forward::GraphicsObject> p)
	{
		assert(p);
		return static_cast<T>(p->DeviceObject().get());
	}

}