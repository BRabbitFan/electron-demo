from conan import ConanFile

class ElectronDemoConan(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  generators = "CMakeDeps"

  def requirements(self):
    self.requires("sdl/3.4.0")
    self.requires("ffmpeg/8.1")
