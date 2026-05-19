# SPDX-FileComment: Conan v2 recipe for the c-gui project
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file conanfile.py
# @brief Dependency management for c-gui
# @version 0.5.0
# @date 2026-05-19
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2026 ZHENG Robert
# @license Apache-2.0

from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy
import os

class CGuiRecipe(ConanFile):
    name = "c-gui"
    version = "0.5.0"
    package_type = "application"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("fontconfig/2.17.1", override=True)
        self.requires("wxwidgets/[>=3.3 <4]")
        self.requires("nlohmann_json/[>=3.12 <4]")
        self.requires("libcurl/[>=8.20 <9]")
        self.requires("libsodium/[>=1.0.21 <2]")
        self.requires("valijson/[>=1.1 <2]")
        self.requires("libpqxx/[>=8.0 <9]")
        self.requires("librdkafka/[>=2.14 <3]")
        self.requires("inih/[>=62]")
        self.requires("spdlog/[>=1.15 <2]")
        self.requires("catch2/[>=3.14 <4]")
        self.requires("cpp-httplib/[>=0.44 <1]")
        self.requires("openxlsx/[>=0.4 <1]")

    def configure(self):
        self.options["cpp-httplib"].with_openssl = True
        self.options["wxwidgets"].shared = True
        if self.settings.os == "Windows":
            self.options["wxwidgets"].msvc_runtime = self.settings.compiler.runtime

    def layout(self):
        cmake_layout(self)

    def generate(self):
        # This copies DLLs to the central 'bin' folder on Windows
        if self.settings.os == "Windows":
            output_dir = os.path.join(self.build_folder, "bin")
            for dep in self.dependencies.values():
                if dep.cpp_info.bindirs:
                    for bindir in dep.cpp_info.bindirs:
                        copy(self, "*.dll", bindir, output_dir)

