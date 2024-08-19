const std = @import("std");

const cflags = [_][]const u8{
    "-std=c++20",
};

const Example = struct {
    name: []const u8,
    files: []const []const u8,
};

const examples = [_]Example{
    .{
        .name = "dx",
        .files = &.{
            "main.cpp",
        },
    },
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const directxmath_dep = b.dependency("directxmath", .{});
    const glew_dep = b.dependency("glew", .{});
    const grapho_dep = b.dependency("grapho", .{});
    const imgui_dep = b.dependency("imgui", .{});

    for (examples) |example| {
        const exe = b.addExecutable(.{
            .name = example.name,
            .target = target,
            .optimize = optimize,
        });
        exe.linkLibCpp();
        exe.addCSourceFiles(.{
            .root = b.path("example/gl3"),
            .files = example.files,
            .flags = &cflags,
        });
        exe.addIncludePath(directxmath_dep.path("Inc"));
        exe.addIncludePath(glew_dep.path("include"));
        exe.addIncludePath(grapho_dep.path("src"));
        exe.addIncludePath(imgui_dep.path(""));
        exe.addIncludePath(b.path("example/bvhutil"));
        exe.addIncludePath(b.path("cuber/include"));

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
