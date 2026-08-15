from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class MicroserviceTemplate(ConanFile):
    name = "cpp-rest-microservice-template"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("drogon/1.9.5")
        self.requires("spdlog/1.14.1")
        self.requires("opentelemetry-cpp/1.16.1")
        self.requires("prometheus-cpp/1.2.4")
        self.requires("catch2/3.7.1")
        self.requires("fakeit/2.4.1")

    def configure(self):
        # Disable unused Drogon back-ends to reduce build time
        self.options["drogon"].with_orm        = False
        self.options["drogon"].with_redis      = False
        self.options["drogon"].with_sqlite3    = False
        self.options["drogon"].with_postgresql = False
        self.options["drogon"].with_mysql      = False

        # Use OTLP HTTP only — avoids pulling in gRPC + protobuf
        self.options["opentelemetry-cpp"].with_otlp_http = True
        self.options["opentelemetry-cpp"].with_otlp_grpc = False

        # core-only; MetricsController serves /metrics via Drogon (avoids port conflict)
        self.options["prometheus-cpp"].with_pull = False
        self.options["prometheus-cpp"].with_push = False

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
