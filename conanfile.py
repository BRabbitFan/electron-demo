from conan import ConanFile
from conan.tools.files import copy, rmdir
from os import path

class ElectronDemoConan(ConanFile):
  settings = 'os', 'compiler', 'build_type', 'arch'
  generators = 'CMakeDeps'

  def requirements(self):
    self.requires('boost/1.90.0')
    self.requires('glm/1.0.1')
    self.requires('nlohmann_json/3.12.0')
    self.requires('sdl/3.4.0')

  def configure(self):
    self.options['boost/*'].header_only = True
    self.options['glm/*'].shared = True
    self.options['nlohmann_json/*'].header_only = True
    self.options['sdl/*'].shared = True

  def generate(self):
    bindir = path.join(self.generators_folder, 'bin')
    rmdir(self, bindir)

    for dep in self.dependencies.values():
      if dep.options.get_safe('shared'):
        for libdir in dep.cpp_info.aggregated_components().libdirs:
          copy(self, '*.so*', src=libdir, dst=bindir)
          copy(self, '*.dylib*', src=libdir, dst=bindir)
          copy(self, '*.dll', src=libdir, dst=bindir)
        for binpath in dep.cpp_info.aggregated_components().bindirs:
          copy(self, '*.dll', src=binpath, dst=bindir)
