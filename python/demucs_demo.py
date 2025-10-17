import demucs.separate
import sys

# Path to your input audio file(s)
input_files = ["./shader_scripts/audio/art.mp3"]  # You can add more files to this list

# Output directory for separated stems
output_dir = "separated_stems"

# Optional: choose a model (e.g., "htdemucs", "mdx_extra", etc.)
model_name = "htdemucs"

# Run separation
demucs.separate.main([
    "-n", model_name,         # Model name
    "-o", output_dir,         # Output directory
    *input_files              # Input files
])

print(f"Separation complete! Check the '{output_dir}' directory.")