const std = @import("std");
const grapho = @import("grapho");

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
    const grapho_opts = grapho.MakeOptions{
        .b = b,
        .target = target,
        .optimize = optimize,
        .dep_builder = grapho_dep.builder,
    };
    // const glfw_dep = grapho_dep.builder.dependency("glfw", .{});
    // const directxmath_dep = b.dependency("directxmath", .{});
    // const glew_dep = b.dependency("glew", .{});
    // const imgui_dep = b.dependency("imgui", .{});
    const asio_dep = b.dependency("asio", .{});
    const meshutils_dep = b.dependency("meshutils", .{});

    for (examples) |example| {
        const exe = b.addExecutable(.{
            .name = example.name,
            .target = target,
            .optimize = optimize,
        });

        // deps
        exe.linkLibCpp();
        grapho.make_grapho(grapho_opts, cflags).inject_to(exe);
        grapho.make_directxmath(grapho_opts).inject_to(exe);
        grapho.make_glew(grapho_opts, cflags).inject_to(exe);
        grapho.make_glfw(grapho_opts, cflags).inject_to(exe);
        grapho.make_imgui(grapho_opts, cflags).inject_to(exe);

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
                "cuber/src/gl3/GlCubeRenderer.cpp",
            },
        });
        exe.addIncludePath(b.path("example/bvhutil"));
        exe.addCSourceFiles(.{
            .root = b.path("example/bvhutil"),
            .files = &bvhutil,
            .flags = &cflags,
        });

        // install
        const install = b.addInstallArtifact(exe, .{});
        b.getInstallStep().dependOn(&install.step);
        // run
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
