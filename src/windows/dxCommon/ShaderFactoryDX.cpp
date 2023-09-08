#pragma warning( disable : 4244 )
#pragma warning( disable : 4239 )
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "ShaderFactoryDX.h"
#include "Log.h"
#include "utilities/Utils.h"
#include "dxCommon/d3dUtil.h"
#include "FileSystem.h"
#include "FileLoader.h"
#include <dxcapi.h>
#include <d3d12shader.h>
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ID3DBlob* ShaderFactoryDX::GenerateShader(const WString&, const String& shaderText, const String& function,
            const String& model, const D3D_SHADER_MACRO* pDefines, bool enablelogging )
{
	HRESULT hr = S_OK;

	std::wstringstream message;

	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrorMessages = nullptr;

	// TODO: The compilation of shaders has to skip the warnings as errors 
	//       for the moment, since the new FXC.exe compiler in VS2012 is
	//       apparently more strict than before.

    u32 flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION; // | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

	if ( FAILED( hr = D3DCompile( 
		shaderText.c_str(),
		shaderText.length(),
		nullptr,
		pDefines,
		nullptr,
		function.c_str(),
		model.c_str(),
		flags,
		0,
		&pCompiledShader,
		&pErrorMessages ) ) )

	{
		message << L"Error compiling shader program" << std::endl;
		message << L"The following error was reported:" << std::endl;

		if ( ( enablelogging ) && ( pErrorMessages != nullptr ) )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			message << TextHelper::ToUnicode( std::string( pMessage ) );
			auto str = message.str();
			Log::Get().Write(str);
		}

		SAFE_RELEASE( pCompiledShader );
		SAFE_RELEASE( pErrorMessages );

		return( nullptr );
	}

	SAFE_RELEASE( pErrorMessages );

	return pCompiledShader;
}
//--------------------------------------------------------------------------------
Vector<u8> ShaderFactoryDX::GenerateShader6(const WString& shaderFileName, const String& shaderText, const String& entry, const String& model, 
    std::function<void(Microsoft::WRL::ComPtr<ID3D12ShaderReflection>)> reflectionCallback)
{
    Vector<u8> resultShader = {};
    // 
    // Create compiler and utils.
    //
    Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

    //
    // Create default include handler. (You can create your own...)
    //
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
    pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

    const auto pdbFolder = FileSystem::getSingleton().GetShaderPDBFolder();
    //
    // COMMAND LINE:
    // dxc myshader.hlsl -E main -T ps_6_0 -Zi -D MYDEFINE=1 -Fo myshader.bin -Fd myshader.pdb -Qstrip_reflect
    //
    const WString entryW = TextHelper::ToUnicode(entry);
    const WString modelW = TextHelper::ToUnicode(model);
    LPCWSTR pszArgs[] =
    {
        shaderFileName.c_str(),            // Optional shader source file name for error reporting and for PIX shader source view.  
        L"-E", entryW.c_str(),                // Entry point.
        L"-T", modelW.c_str(),              // Target.
        L"-Zi",                                      // Enable debug information (slim format)
        L"-Fd", pdbFolder.c_str(),
        L"-Zpr",                              //	Pack matrices in row - major order
#ifdef _DEBUG
        L"-Od",
#endif
        //L"-D", L"MYDEFINE=1",        // A single define.
        L"-Qstrip_reflect",          // Strip reflection into a separate blob. 
        L"-Qstrip_debug"
    };

    DxcBuffer Source;
    Source.Ptr = shaderText.c_str();
    Source.Size = shaderText.length();
    Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

    //
    // Compile it with specified arguments.
    //
    Microsoft::WRL::ComPtr<IDxcResult> pResults;
    pCompiler->Compile(
        &Source,                     // Source buffer.
        pszArgs,                      // Array of pointers to arguments.
        _countof(pszArgs),      // Number of arguments.
        pIncludeHandler.Get(),       // User-provided interface to handle #include directives (optional).
        IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
    );

    //
    // Print errors if present.
    //
    Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
    pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
    // Note that d3dcompiler would return null if no errors or warnings are present.
    // IDxcCompiler3::Compile will always return an error buffer, but its length
    // will be zero if there are no warnings or errors.
    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
    {
        std::wstringstream ss;
        ss << "Warnings and Errors:\n" << pErrors->GetStringPointer() << std::endl;
        auto str = ss.str();
        Log::Get().Write(str);
    }

    //
    // Quit if the compilation failed.
    //
    HRESULT hrStatus;
    pResults->GetStatus(&hrStatus);
    if (FAILED(hrStatus))
    {
        Log::Get().Write(L"Compilation Failed\n");
        return resultShader;
    }

    //
    // Save shader binary.
    //
    Microsoft::WRL::ComPtr<IDxcBlob> pShader = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
    pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);
    if (pShader != nullptr)
    {
        resultShader.resize(pShader->GetBufferSize());
        memcpy(resultShader.data(), pShader->GetBufferPointer(), pShader->GetBufferSize());
    }

    //
    // Save pdb.
    //
    Microsoft::WRL::ComPtr<IDxcBlob> pPDB = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
    pResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
    {
        FILE* fp = NULL;

        WString pdbPath = pdbFolder + pPDBName->GetStringPointer();
        // Note that if you don't specify -Fd, a pdb name will be automatically generated.
        // Use this file name to save the pdb so that PIX can find it quickly.
        _wfopen_s(&fp, pdbPath.c_str(), L"wb");
        fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp);
        fclose(fp);
    }

    //
    // Get separate reflection.
    //
    Microsoft::WRL::ComPtr<IDxcBlob> pReflectionData;
    pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
    if (pReflectionData != nullptr)
    {
        // Create reflection interface.
        DxcBuffer ReflectionData;
        ReflectionData.Encoding = DXC_CP_ACP;
        ReflectionData.Ptr = pReflectionData->GetBufferPointer();
        ReflectionData.Size = pReflectionData->GetBufferSize();

        Microsoft::WRL::ComPtr< ID3D12ShaderReflection > pReflection;
        pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&pReflection));
        reflectionCallback(pReflection);
    }

    std::wstringstream ss;
    ss << "Compile shader : " << shaderFileName << " succeeded" << std::endl;
    auto str = ss.str();
    Log::Get().Write(str);
    return resultShader;
}