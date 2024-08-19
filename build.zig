const std = @import("std");

const cflags = [_][]const u8{
    "-std=c++20",
};

const Example = struct {
    name: []const u8,
    root: []const u8,
    files: []const []const u8,
};

const examples = [_]Example{
    .{
        .name = "gl3",
        .root = "example/gl3",
        .files = &.{
            "main.cpp",
            "GlfwPlatform.cpp",
        },
    },
};

const bvhutil = [_][]const u8{
    "GuiApp.cpp",
    "Bvh.cpp",
    "BvhSolver.cpp",
    "BvhNode.cpp",
    "Animation.cpp",
    "UdpSender.cpp",
    "BvhPanel.cpp",
    "Payload.cpp",
    "BvhFrame.cpp",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const grapho_dep = b.dependency("grapho", .{});
    const glfw_dep = grapho_dep.builder.dependency("glfw", .{});
    const directxmath_dep = b.dependency("directxmath", .{});
    const glew_dep = b.dependency("glew", .{});
    const imgui_dep = b.dependency("imgui", .{});

    const asio_dep = b.dependency("asio", .{});
    const meshutils_dep = b.dependency("meshutils", .{});

    for (examples) |example| {
        const exe = b.addExecutable(.{
            .name = example.name,
            .target = target,
            .optimize = optimize,
        });
        exe.linkLibCpp();
        exe.addCSourceFiles(.{
            .root = b.path(example.root),
            .files = example.files,
            .flags = &cflags,
        });
        exe.addIncludePath(directxmath_dep.path("Inc"));
        exe.addIncludePath(glew_dep.path("include"));
        exe.addIncludePath(grapho_dep.path("src"));
        exe.addIncludePath(imgui_dep.path(""));
        exe.addIncludePath(imgui_dep.path("backends"));
        exe.addIncludePath(asio_dep.path("asio/include"));
        exe.addIncludePath(meshutils_dep.path("Source"));
        exe.addIncludePath(glfw_dep.path("include"));

        exe.addIncludePath(b.path("cuber/include"));
        exe.addCSourceFiles(.{
            .files = &.{
                "cuber/src/gl3/GlCubeRenderer.cpp",
            },
        });

        exe.addIncludePath(b.path("example/bvhutil"));
        exe.addCSourceFiles(.{
            .root = b.path("example/bvhutil"),
            .files = &bvhutil,
            .flags = &cflags,
        });

        const install = b.addInstallArtifact(exe, .{});
        b.getInstallStep().dependOn(&install.step);

        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }
        const run_step = b.step(
            b.fmt("run-{s}", .{example.name}),
            b.fmt("Run the {s}", .{example.name}),
        );
        run_step.dependOn(&run_cmd.step);
    }
}
