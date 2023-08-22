#include "PCH.h"
#include "FileSystem.h"
#include <iostream>

#include <MaterialXRender/LightHandler.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/ShaderTranslator.h>
#define MATERIALX_GENHLSL_EXPORTS
#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXFormat/Environ.h>
#include <MaterialXFormat/Util.h>

using namespace forward;
using namespace MaterialX;
namespace mx = MaterialX;

DocumentPtr _stdLib;
FileSearchPath _searchPath;
FilePathVec _libraryFolders;
UnitConverterRegistryPtr _unitRegistry;

FileSearchPath getDefaultSearchPath()
{
    FilePath modulePath = FilePath::getModulePath();
    FilePath installRootPath = modulePath.getParentPath();
    FilePath devRootPath = installRootPath.getParentPath().getParentPath().getParentPath().getParentPath().getParentPath();

    FileSearchPath searchPath;
    if ((devRootPath / "MaterialX_JHQ" / "MaterialX").exists())
    {
        searchPath.append(devRootPath / "MaterialX_JHQ" / "MaterialX");
    }
    searchPath.append(devRootPath / "MaterialX_JHQ" / "MaterialX" / "source" / "MaterialXGenHlsl");

    return searchPath;
}

void initContext(GenContext& context)
{
    // Initialize search path.
    context.registerSourceCodeSearchPath(_searchPath);

    // Initialize color management.
    DefaultColorManagementSystemPtr cms = DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    cms->loadLibrary(_stdLib);
    context.getShaderGenerator().setColorManagementSystem(cms);

    // Initialize unit management.
    UnitSystemPtr unitSystem = UnitSystem::create(context.getShaderGenerator().getTarget());
    unitSystem->loadLibrary(_stdLib);
    unitSystem->setUnitConverterRegistry(_unitRegistry);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    context.getOptions().targetDistanceUnit = "meter";
}

i32 main()
{
	FileSystem fileSystem;
	const std::string materialFileName = "D:\\Downloads\\Midnite_Fleece_Fabric_1k_8b\\Midnite_Fleece_Fabric.mtlx";

	GenContext _genContextHlsl = HlslShaderGenerator::create();
	_genContextHlsl.getOptions().targetColorSpaceOverride = "lin_rec709";
	_genContextHlsl.getOptions().fileTextureVerticalFlip = false;

    _searchPath = getDefaultSearchPath();
    _libraryFolders.push_back("libraryHlsl");
    _libraryFolders.push_back("libraries");
	_stdLib = createDocument();
	auto _xincludeFiles = loadLibraries(_libraryFolders, _searchPath, _stdLib);
	assert(!_xincludeFiles.empty());

    _unitRegistry = mx::UnitConverterRegistry::create();

    // Initialize unit management.
    mx::UnitTypeDefPtr distanceTypeDef = _stdLib->getUnitTypeDef("distance");
    mx::LinearUnitConverterPtr _distanceUnitConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    _unitRegistry->addUnitConverter(distanceTypeDef, _distanceUnitConverter);
    mx::UnitTypeDefPtr angleTypeDef = _stdLib->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    _unitRegistry->addUnitConverter(angleTypeDef, angleConverter);

    // Create the list of supported distance units.
    auto unitScales = _distanceUnitConverter->getUnitScale();
    mx::StringVec _distanceUnitOptions;
    _distanceUnitOptions.resize(unitScales.size());
    for (auto unitScale : unitScales)
    {
        int location = _distanceUnitConverter->getUnitAsInteger(unitScale.first);
        _distanceUnitOptions[location] = unitScale.first;
    }

    initContext(_genContextHlsl);

    // load document
    // Set up read options.
    mx::XmlReadOptions readOptions;
    readOptions.readXIncludeFunction = [](mx::DocumentPtr doc, const mx::FilePath& filename,
        const mx::FileSearchPath& searchPath, const mx::XmlReadOptions* options)
        {
            mx::FilePath resolvedFilename = searchPath.find(filename);
            if (resolvedFilename.exists())
            {
                readFromXmlFile(doc, resolvedFilename, searchPath, options);
            }
            else
            {
                std::cerr << "Include file not found: " << filename.asString() << std::endl;
            }
        };

    // Load source document.
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, materialFileName, _searchPath, &readOptions);
    //_materialSearchPath = mx::getSourceSearchPath(doc);

    // Import libraries.
    doc->importLibrary(_stdLib);

    // applyDirectLights
    mx::FilePath _lightRigFilename = "D:\\Documents\\GitHub\\MaterialX_JHQ\\MaterialX\\resources\\Lights\\san_giuseppe_bridge_split.mtlx";
    mx::DocumentPtr _lightRigDoc = mx::createDocument();
    mx::readFromXmlFile(_lightRigDoc, _lightRigFilename, _searchPath);

    doc->importLibrary(_lightRigDoc);

    std::vector<mx::NodePtr> lights;
    mx::LightHandlerPtr _lightHandler = mx::LightHandler::create();
    _lightHandler->findLights(doc, lights);
    _lightHandler->registerLights(doc, lights, _genContextHlsl);
    _lightHandler->setLightSources(lights);

    std::vector<mx::TypedElementPtr> elems;
    mx::findRenderableElements(doc, elems);
    assert(elems.size() == 1);
    auto shader = _genContextHlsl.getShaderGenerator().generate("MaterialXTest_JHQ", elems[0], _genContextHlsl);
    const std::string& pixelShader = shader->getSourceCode(mx::Stage::PIXEL);
    const std::string& vertexShader = shader->getSourceCode(mx::Stage::VERTEX);

    const auto& ps = shader->getStage(mx::Stage::PIXEL);
    // Process pixel stage uniforms
    for (const auto& uniformMap : ps.getUniformBlocks())
    {
        const mx::VariableBlock& uniforms = *uniformMap.second;
        for (size_t i = 0; i < uniforms.size(); ++i)
        {
            const mx::ShaderPort* v = uniforms[i];
            if (v->getValue())
                std::cout << v->getName() << "[" << v->getType()->getName() << "] : " << v->getValue()->getValueString() << std::endl;
        }
    }

	return 0;
}