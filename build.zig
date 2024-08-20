const std = @import("std");
const grapho = @import("grapho");

const cflags = [_][]const u8{
    "-std=c++20",
    "-DGLEW_STATIC",
};

const libs = [_][]const u8{
    "gdi32",
    "OpenGL32",
    "Ws2_32",
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
            "run.cpp",
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
    const asio_dep = b.dependency("asio", .{});
    const meshutils_dep = b.dependency("meshutils", .{});

    // deps
    const gb = grapho_dep.builder;
    const directxmath = grapho.CLib.make_headeronly(
        gb.dependency("directxmath", .{}),
        &.{"Inc"},
    );
    const glew = grapho.make_glew(b, gb.dependency("glew", .{}), target, optimize);
    const glfw = grapho.make_glfw(b, gb.dependency("glfw", .{}), target, optimize);
    const imgui = grapho.make_imgui(b, gb.dependency("imgui", .{}), target, optimize);
    const grapho_lib = grapho.make_grapho(b, gb.dependency("grapho", .{}), target, optimize);
    if (grapho_lib.lib) |lib| {
        directxmath.injectIncludes(lib);
        glfw.injectIncludes(lib);
        glew.injectIncludes(lib);
    }
    if (imgui.lib) |lib| {
        glfw.injectIncludes(lib);
    }

    const dep_sokol = b.dependency("sokol", .{
        .target = target,
        .optimize = optimize,
    });

    for (examples) |example| {
        const exe = b.addExecutable(.{
            .name = example.name,
            .target = target,
            .optimize = optimize,
            .link_libc = true,
            .root_source_file = b.path("example/gl3/main.zig"),
        });
        exe.linkLibCpp();
        exe.root_module.addImport("sokol", dep_sokol.module("sokol"));
        // srcs
        exe.addCSourceFiles(.{
            .root = b.path(example.root),
            .files = example.files,
            .flags = &cflags,
        });
        exe.addIncludePath(asio_dep.path("asio/include"));
        exe.addIncludePath(meshutils_dep.path("Source"));
        exe.addIncludePath(b.path("cuber/include"));
        exe.addCSourceFiles(.{
            .files = &.{
                "cuber/src/mesh.cpp",
                "cuber/src/gl3/GlCubeRenderer.cpp",
                "cuber/src/gl3/GlLineRenderer.cpp",
            },
            .flags = &cflags,
        });
        exe.addIncludePath(b.path("example/bvhutil"));
        exe.addCSourceFiles(.{
            .root = b.path("example/bvhutil"),
            .files = &bvhutil,
            .flags = &cflags,
        });

        grapho_lib.injectIncludes(exe);
        directxmath.injectIncludes(exe);
        glew.injectIncludes(exe);
        glfw.injectIncludes(exe);
        imgui.injectIncludes(exe);
        if (grapho_lib.lib) |lib| {
            exe.linkLibrary(lib);
        }
        if (imgui.lib) |lib| {
            exe.linkLibrary(lib);
        }
        if (glfw.lib) |lib| {
            exe.linkLibrary(lib);
        }
        if (glew.lib) |lib| {
            exe.linkLibrary(lib);
        }
        for (libs) |lib| {
            exe.linkSystemLibrary(lib);
        }

        // install
        const install = b.addInstallArtifact(exe, .{});
        b.getInstallStep().dependOn(&install.step);
        // run
        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(&install.step);
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
