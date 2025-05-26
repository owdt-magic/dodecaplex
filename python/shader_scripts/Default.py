import moderngl_window as mglw
import pyglet                                    # ← add this
                                             
class UniformImporter(mglw.WindowConfig):
    window_size   = 1920, 1080
    resource_dir  = 'programs'
    title         = "Shader Viewer with FPS"
    gl_version    = 3, 3                        # keep pyglet happy on macOS

    # ------------------------------------------------------------------ #
    # 1 new instance variable  ---------------------------------------- #
    show_fps = True                            # ⟵ toggle with the F-key
    # ------------------------------------------------------------------ #

    def __init__(self, **kw):
        super().__init__(**kw)
        self.u_scroll  = 3.0
        self.quad      = mglw.geometry.quad_fs()
        self.establish_shader()

    # -------------------------------------------------------- ONE METHOD
    def track_fps(self):
        """If show_fps is enabled, overlay the current FPS using pyglet."""
        if not self.show_fps:
            return
        fps = self.timer.fps                   # moderngl-window’s live value :contentReference[oaicite:0]{index=0}
        # draw in the upper-left corner (OpenGL origin is bottom-left)
        pyglet.text.Label(
            f"FPS: {fps:5.1f}",
            font_size = 14,
            x         = 10,
            y         = self.wnd.buffer_size[1] - 20,
            anchor_x  = 'left',
            anchor_y  = 'top',
            color     = (255, 255, 255, 255)
        ).draw()                               # pyglet label draw :contentReference[oaicite:1]{index=1}

    # ------------------------------------------------------------------ #

    def establish_shader(self):
        assert self.vertex_shader and self.fragment_shader
        self.program = self.load_program(
            vertex_shader   = self.vertex_shader,
            fragment_shader = self.fragment_shader
        )
        if 'u_resolution' in self.program:
            self.program['u_resolution'] = self.window_size
        if 'u_scroll' in self.program:
            self.program['u_scroll'] = self.u_scroll

    # ----------------------------- main render loop ------------------ #
    def on_render(self, time, frame_time):
        self.ctx.clear()
        if 'u_time' in self.program:
            self.program['u_time'] = time
        self.quad.render(self.program)

        # overlay FPS (last thing you draw each frame)
        self.track_fps()

    # ----------------------------- input handlers -------------------- #
    def mouse_position_event(self, x, y, dx, dy):
        if 'u_mouse' in self.program:
            self.program['u_mouse'] = (x, y)

    def mouse_scroll_event(self, x_offset, y_offset):
        self.u_scroll = max(1.0, self.u_scroll + y_offset)
        if 'u_scroll' in self.program:
            self.program['u_scroll'] = self.u_scroll

    def key_event(self, key, action, modifiers):
        if action == self.wnd.keys.ACTION_PRESS:
            if key == self.wnd.keys.SPACE:
                print("SPACE key was pressed")
                self.establish_shader()
            elif key == self.wnd.keys.F:               # toggle display
                self.show_fps = not self.show_fps
                print(f"FPS overlay {'ON' if self.show_fps else 'OFF'}")
