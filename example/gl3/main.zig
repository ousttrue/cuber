const sokol = @import("sokol");
const sg = sokol.gfx;

extern fn gl3_init() void;
extern fn gl3_frame() void;

var pass_action = sg.PassAction{};

export fn init() void {
    sg.setup(.{
        .environment = sokol.glue.environment(),
        .logger = .{ .func = sokol.log.func },
    });
    pass_action.colors[0] = .{
        .load_action = .CLEAR,
        .clear_value = .{ .r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0 },
    };
    // __dbgui_setup(sapp_sample_count());
    gl3_init();
}

export fn frame() void {
    gl3_frame();

    const g = pass_action.colors[0].clear_value.g + 0.01;
    pass_action.colors[0].clear_value.g = if (g > 1.0) 0.0 else g;
    sg.beginPass(.{
        .action = pass_action,
        .swapchain = sokol.glue.swapchain(),
    });
    // __dbgui_draw();
    sg.endPass();
    sg.commit();
}

export fn cleanup() void {
    // __dbgui_shutdown();
    sg.shutdown();
}

pub fn main() void {
    sokol.app.run(.{
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        // .event_cb = __dbgui_event,
        .width = 1024,
        .height = 768,
        .window_title = "cuber(sokol-zig)",
        .icon = .{ .sokol_default = true },
        .logger = .{ .func = sokol.log.func },
    });
}
