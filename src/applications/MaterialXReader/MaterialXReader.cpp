#include "MaterialXReader.h"
#include <iostream>
#include <assert.h>

#include <MaterialXRender/LightHandler.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/ShaderTranslator.h>
#define MATERIALX_GENHLSL_EXPORTS
#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXFormat/Environ.h>
#include <MaterialXFormat/Util.h>

using namespace MaterialX;
namespace mx = MaterialX;

FileSearchPath GetDefaultSearchPath(FilePath& outMaterialXRoot)
{
    FilePath modulePath = FilePath::getModulePath();
    FilePath forwardRootPath = modulePath;
    while (forwardRootPath.getBaseName() != "forward")
        forwardRootPath = forwardRootPath.getParentPath();
    FilePath materialXRootPath = forwardRootPath / "extern" / "src" / "MaterialX";
    outMaterialXRoot = materialXRootPath;

    FileSearchPath searchPath;
    if (materialXRootPath.exists())
    {
        searchPath.append(materialXRootPath);
    }
    searchPath.append(materialXRootPath / "source" / "MaterialXGenHlsl");

    return searchPath;
}

void InitContext(GenContext& context, DocumentPtr stdLib, FileSearchPath searchPath, UnitConverterRegistryPtr unitRegistry)
{
    // Initialize search path.
    context.registerSourceCodeSearchPath(searchPath);

    // Initialize color management.
    DefaultColorManagementSystemPtr cms = DefaultColorManagementSystem::create(context.getShaderGenerator().getTarget());
    cms->loadLibrary(stdLib);
    context.getShaderGenerator().setColorManagementSystem(cms);

    // Initialize unit management.
    UnitSystemPtr unitSystem = UnitSystem::create(context.getShaderGenerator().getTarget());
    unitSystem->loadLibrary(stdLib);
    unitSystem->setUnitConverterRegistry(unitRegistry);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    context.getOptions().targetDistanceUnit = "meter";
}

int Forward_Read_MaterialX(const char* file, std::string& outVS, std::string& outPS, 
    std::unordered_map<std::string, std::string>& paramsPS)
{
	GenContext genContextHlsl = HlslShaderGenerator::create();
	genContextHlsl.getOptions().targetColorSpaceOverride = "lin_rec709";
	genContextHlsl.getOptions().fileTextureVerticalFlip = false;

    FilePath materialXRoot;
    FileSearchPath searchPath = GetDefaultSearchPath(materialXRoot);
    FilePathVec libraryFolders;
    libraryFolders.push_back("libraryHlsl");
    libraryFolders.push_back("libraries");
	DocumentPtr stdLib = createDocument();
	auto _xincludeFiles = loadLibraries(libraryFolders, searchPath, stdLib);
	assert(!_xincludeFiles.empty());

    UnitConverterRegistryPtr unitRegistry = mx::UnitConverterRegistry::create();

    // Initialize unit management.
    mx::UnitTypeDefPtr distanceTypeDef = stdLib->getUnitTypeDef("distance");
    mx::LinearUnitConverterPtr _distanceUnitConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    unitRegistry->addUnitConverter(distanceTypeDef, _distanceUnitConverter);
    mx::UnitTypeDefPtr angleTypeDef = stdLib->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    unitRegistry->addUnitConverter(angleTypeDef, angleConverter);

    // Create the list of supported distance units.
    auto unitScales = _distanceUnitConverter->getUnitScale();
    mx::StringVec _distanceUnitOptions;
    _distanceUnitOptions.resize(unitScales.size());
    for (auto unitScale : unitScales)
    {
        int location = _distanceUnitConverter->getUnitAsInteger(unitScale.first);
        _distanceUnitOptions[location] = unitScale.first;
    }

    InitContext(genContextHlsl, stdLib, searchPath, unitRegistry);

    // load document
    // Set up read options.
    mx::XmlReadOptions readOptions;
    readOptions.readXIncludeFunction = [](mx::DocumentPtr doc, const mx::FilePath& filename,
        const mx::FileSearchPath& searchPath, const mx::XmlReadOptions* options)
        {
            mx::FilePath resolvedFilename = searchPath.find(filename);
            if (resolvedFilename.exists())
                readFromXmlFile(doc, resolvedFilename, searchPath, options);
            else
                std::cerr << "Include file not found: " << filename.asString() << std::endl;
        };

    // Load source document.
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, file, searchPath, &readOptions);
    //_materialSearchPath = mx::getSourceSearchPath(doc);

    // Import libraries.
    doc->importLibrary(stdLib);

    // applyDirectLights
    mx::FilePath _lightRigFilename = materialXRoot / "resources" / "Lights" / "san_giuseppe_bridge_split.mtlx";
    mx::DocumentPtr _lightRigDoc = mx::createDocument();
    mx::readFromXmlFile(_lightRigDoc, _lightRigFilename, searchPath);

    doc->importLibrary(_lightRigDoc);

    std::vector<mx::NodePtr> lights;
    mx::LightHandlerPtr _lightHandler = mx::LightHandler::create();
    _lightHandler->findLights(doc, lights);
    _lightHandler->registerLights(doc, lights, genContextHlsl);
    _lightHandler->setLightSources(lights);

    std::vector<mx::TypedElementPtr> elems;
    mx::findRenderableElements(doc, elems);
    assert(elems.size() == 1);
    auto shader = genContextHlsl.getShaderGenerator().generate("MaterialXTest_JHQ", elems[0], genContextHlsl);
    outPS = shader->getSourceCode(mx::Stage::PIXEL);
    outVS = shader->getSourceCode(mx::Stage::VERTEX);

    const auto& vs = shader->getStage(mx::Stage::VERTEX);
    for (const auto& uniformMap : vs.getUniformBlocks())
    {
        const mx::VariableBlock& uniforms = *uniformMap.second;
        std::string prefix = uniformMap.first + "_vertex.";
        for (size_t i = 0; i < uniforms.size(); ++i)
        {
            const mx::ShaderPort* v = uniforms[i];
            if (v->getValue())
                paramsPS[prefix + v->getName()] = v->getValue()->getValueString();
        }
    }

    const auto& ps = shader->getStage(mx::Stage::PIXEL);
    for (const auto& uniformMap : ps.getUniformBlocks())
    {
        const mx::VariableBlock& uniforms = *uniformMap.second;
        std::string prefix = uniformMap.first + "_pixel.";
        for (size_t i = 0; i < uniforms.size(); ++i)
        {
            const mx::ShaderPort* v = uniforms[i];
            if (v->getValue())
                paramsPS[prefix + v->getName()] = v->getValue()->getValueString();
        }
    }

	return 0;
}