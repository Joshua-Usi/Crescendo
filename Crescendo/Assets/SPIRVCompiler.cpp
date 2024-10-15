#include "SPIRVCompiler.hpp"
#include "glslang/Include/ShaderLang.h"
#include "glslang/Include/intermediate.h"
#include "glslang/GlslangToSpv.h"
#include "tcpp/tcppLibrary.hpp"
#include "utils/utils.hpp"
#include <mutex>

constexpr TBuiltInResource DEFAULT_BUILT_IN_RESOURCE_LIMIT
{
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,
	/* maxMeshOutputVerticesEXT;*/256,
	/* maxMeshOutputPrimitivesEXT;*/512,
	/* maxMeshWorkGroupSizeX_EXT;*/32,
	/* maxMeshWorkGroupSizeY_EXT;*/1,
	/* maxMeshWorkGroupSizeZ_EXT;*/32,
	/* maxTaskWorkGroupSizeX_EXT;*/1,
	/* maxTaskWorkGroupSizeY_EXT;*/1,
	/* maxTaskWorkGroupSizeZ_EXT;*/1,
	/* maxMeshViewCountEXT;*/1,
	/* .maxDualSourceDrawBuffersEXT = */1,
	/* .limits = */
	{
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};

CS_NAMESPACE_BEGIN
{
	static std::once_flag glslangInitFlag;

	EShLanguage GetLanguageFromExtension(const std::filesystem::path& file)
	{
		if (file.extension() == ".vert")
			return EShLangVertex;
		if (file.extension() == ".frag")
			return EShLangFragment;
		if (file.extension() == ".comp")
			return EShLangCompute;
		if (file.extension() == ".geom")
			return EShLangGeometry;
		if (file.extension() == ".tesc")
			return EShLangTessControl;
		if (file.extension() == ".tese")
			return EShLangTessEvaluation;
		// default to vertex shader
		return EShLangVertex;
	}

	std::vector<uint8_t> GLSLToSPIRV(const std::filesystem::path& file)
	{
		std::call_once(glslangInitFlag, []() {
			if (!glslang::InitializeProcess()) {
				cs_std::console::error("Failed to initialize glslang.");
				throw std::runtime_error("glslang initialization failed");
			}
		});

		const TBuiltInResource* resources = &DEFAULT_BUILT_IN_RESOURCE_LIMIT;

		std::string source = GLSLPreprocessorIncludes(file);
		auto inputStream = std::make_unique<tcpp::StringInputStream>(source);
		tcpp::Lexer lexer(std::move(inputStream));

		tcpp::Preprocessor::TPreprocessorConfigInfo config;
		config.mOnErrorCallback = [](const tcpp::TErrorInfo& errorInfo) {
			cs_std::console::error("Error: ", tcpp::ErrorTypeToString(errorInfo.mType), " at line ", errorInfo.mLine);
		};
		config.mSkipComments = true;

		tcpp::Preprocessor preprocessor(lexer, config);

		// custom directive handlers for GLSL directives
		// remove version directive
		preprocessor.AddCustomDirectiveHandler("version", [](tcpp::Preprocessor& preprocessor, tcpp::Lexer& lexer, const std::string& processedStr) {
			tcpp::TToken currToken;
			while ((currToken = lexer.GetNextToken()).mType != tcpp::E_TOKEN_TYPE::NEWLINE && currToken.mType != tcpp::E_TOKEN_TYPE::END);
			return "";
		});

		// keep extension directives
		preprocessor.AddCustomDirectiveHandler("extension", [](tcpp::Preprocessor& preprocessor, tcpp::Lexer& lexer, const std::string& processedStr) {
			std::string directiveLine = "#extension";
			tcpp::TToken currToken;
			while ((currToken = lexer.GetNextToken()).mType != tcpp::E_TOKEN_TYPE::NEWLINE && currToken.mType != tcpp::E_TOKEN_TYPE::END)
				directiveLine += currToken.mRawView;
			directiveLine += "\n";
			return directiveLine;
		});

		std::string preprocessedSource = preprocessor.Process();
		const char* preprocessedShaderSource = preprocessedSource.c_str();

		EShLanguage language = GetLanguageFromExtension(file);

		glslang::TShader shader(language);

		// Configure the environment for Vulkan
		shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientVulkan, 460);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);
		shader.setEntryPoint("main");

		shader.setStrings(&preprocessedShaderSource, 1);

		// Parse the preprocessed source
		if (!shader.parse(resources, 460, ECoreProfile, false, true, EShMsgEnhanced))
		{
			cs_std::console::error("Failed to parse shader.");
			cs_std::console::log("Parser info log: ", shader.getInfoLog());
		}

		// Link the shader program
		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(EShMsgEnhanced))
		{
			cs_std::console::error("Failed to link program.");
			cs_std::console::log("Info: ", program.getInfoLog());
			cs_std::console::log("Debug Info: ", program.getInfoDebugLog());
		}

		glslang::SpvOptions options {
			.generateDebugInfo = false,
			.stripDebugInfo = false,
			.disableOptimizer = false,
			.optimizeSize = false,
			.disassemble = false,
			.validate = false,
			.emitNonSemanticShaderDebugInfo = false,
			.emitNonSemanticShaderDebugSource = false,
			.compileOnly = false,
			.optimizerAllowExpandedIDBound = false
		};

		std::vector<uint32_t> spirv;
		glslang::GlslangToSpv(*program.getIntermediate(language), spirv, &options);

		 // glslang::FinalizeProcess();

		std::vector<uint8_t> result;
		result.resize(spirv.size() * sizeof(uint32_t));
		std::memcpy(result.data(), spirv.data(), result.size());

		return result;
	}
}