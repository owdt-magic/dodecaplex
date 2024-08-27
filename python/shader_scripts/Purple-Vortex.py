import moderngl_window as mglw
from Default import UniformImporter

class App(UniformImporter):
    window_size = 1600, 900
    resource_dir = 'programs'

    def __init__(self, **kwargs):
        self.vertex_shader = 'vertex.glsl'
        self.fragment_shader = 'purple-vortex.glsl'
        
        super().__init__(**kwargs)

if __name__ == '__main__':
    mglw.run_window_config(App)
