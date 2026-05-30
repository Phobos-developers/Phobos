[1mdiff --git a/Phobos.props b/Phobos.props[m
[1mindex bee83b6d..72f1c3c7 100644[m
[1m--- a/Phobos.props[m
[1m+++ b/Phobos.props[m
[36m@@ -31,6 +31,7 @@[m
     <IntDir>$(Configuration)\IntDir\</IntDir>[m
     <CoreLibraryDependencies></CoreLibraryDependencies>[m
     <PhobosExtraLibs>dbghelp.lib;onecore.lib</PhobosExtraLibs>[m
[32m+[m[32m    <VcpkgTriplet>x86-windows-static</VcpkgTriplet>[m
   </PropertyGroup>[m
   <!-- Global -->[m
   <ItemDefinitionGroup>[m
[36m@@ -52,11 +53,13 @@[m
       <LanguageStandard>stdcpp20</LanguageStandard>[m
       <AssemblerListingLocation>$(IntDir)\%(Directory)</AssemblerListingLocation>[m
       <ObjectFileName>$(IntDir)\%(Directory)</ObjectFileName>[m
[31m-      <CallingConvention>StdCall</CallingConvention>[m
[32m+[m[32m      <CallingConvention>Cdecl</CallingConvention>[m
       <DisableSpecificWarnings>4100;4201;4530;4731;4740;4458;4819;5103;5105</DisableSpecificWarnings>[m
       <EnableModules>true</EnableModules>[m
       <MultiProcessorCompilation>true</MultiProcessorCompilation>[m
       <OpenMPSupport>true</OpenMPSupport>[m
[32m+[m[32m      <AdditionalIncludeDirectories>$(MSBuildThisFileDirectory)vcpkg\installed\x86-windows-static\include;$(MSBuildThisFileDirectory)vcpkg\installed\x86-windows-static\include\harfbuzz;$(MSBuildThisFileDirectory)vcpkg\installed\x86-windows-static\include\freetype;$(MSBuildThisFileDirectory)vcpkg\installed\x86-windows-static\include\fribidi;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>[m
[32m+[m[32m      <AdditionalOptions>/external:env:INCLUDE /external:env:LIB %(AdditionalOptions)</AdditionalOptions>[m
     </ClCompile>[m
     <Link>[m
       <EnableCOMDATFolding>true</EnableCOMDATFolding>[m
[36m@@ -66,7 +69,8 @@[m
       <ProgramDatabaseFile>$(OutDir)$(TargetName).pdb</ProgramDatabaseFile>[m
       <ProfileGuidedDatabase>$(IntDir)$(TargetName).pgd</ProfileGuidedDatabase>[m
       <ImportLibrary>$(IntDir)$(TargetName).lib</ImportLibrary>[m
[31m-    </Link>[m
[32m+[m[32m      <AdditionalLibraryDirectories>$(MSBuildThisFileDirectory)vcpkg\installed\x86-windows-static\lib;$(MSBuildThisFileDirectory)vcpkg\installed\x86-windows-static\lib\manual-link;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>[m
[32m+[m[32m<AdditionalDependencies>freetype.lib;harfbuzz.lib;harfbuzz-subset.lib;fribidi.lib;brotlicommon.lib;brotlidec.lib;bz2.lib;libpng16.lib;zs.lib;%(AdditionalDependencies)</AdditionalDependencies>    </Link>[m
   </ItemDefinitionGroup>[m
   <!-- Release -->[m
   <ItemDefinitionGroup Condition="$(Configuration.Contains('Release'))">[m
