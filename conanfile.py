from conans import ConanFile

class EEUQ(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    version = "1.1.0"
    license = "BSD"
    author = "NHERI SimCenter"
    url = "https://github.com/NHERI-SimCenter/PBE"
    description = "NHERI SimCenter Earthquake Engineering with Uncertainty Quantification Research Tool"
    settings = "os", "compiler", "build_type", "arch"
    generators = "qmake"
    requires = "lapack/3.7.1@conan/stable"

    build_policy = "missing"

    def configure(self):
        if self.settings.os == "Windows":
            self.options["libcurl"].with_winssl = True
            self.options["libcurl"].with_openssl = False
            self.options["lapack"].visual_studio = True
            self.options["lapack"].shared = True

    def imports(self):
        if self.settings.os == "Windows":
            output = './%s' % self.settings.build_type
            self.copy("lib*.dll", dst=output, src="bin")
            self.copy("*", dst='{}/resources'.format(output), src="resources")
