import moderngl_window as mglw

class UniformImporter(mglw.WindowConfig):
    window_size = 1600, 900
    resource_dir = 'programs'

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.u_scroll = 3.0
        self.quad = mglw.geometry.quad_fs()
        self.establish_shader()

    def establish_shader(self):
        assert self.vertex_shader and self.fragment_shader
        self.program = self.load_program(vertex_shader=self.vertex_shader, \
            fragment_shader=self.fragment_shader)
        # uniforms
        if 'u_resolution' in self.program:
            self.program['u_resolution'] = self.window_size
        if 'u_scroll' in self.program:
            self.program['u_scroll'] = self.u_scroll

    def render(self, time, frame_time):
        self.ctx.clear()
        if 'u_time' in self.program:
            self.program['u_time'] = time        
        self.quad.render(self.program)

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