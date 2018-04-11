cmake_minimum_required(VERSION 3.10)

project(FontBMPGenerator VERSION 0.1.0 LANGUAGES CSharp)

include(CSharpUtilities)

add_executable(FontBMPGenerator
	#App.config
    Program.cs
	#Properties/AssemblyInfo.cs
	)

set_property(TARGET FontBMPGenerator PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.6.1")
set_property(TARGET FontBMPGenerator PROPERTY WIN32_EXECUTABLE FALSE)

set_property(TARGET FontBMPGenerator PROPERTY VS_DOTNET_REFERENCES
	"Microsoft.CSharp"
	"System"
	"System.Core"
	"System.Data"
	"System.Data.DataSetExtensions"
	"System.Runtime.InteropServices"
	"System.Net.Http"
	"System.Xml"
	"System.Xml.Linq"
	"System.Drawing")

set_target_properties(FontBMPGenerator PROPERTIES FOLDER Tools)