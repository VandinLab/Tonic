import os
import argparse
import subprocess

def parse_args():
    """
    Parses command-line arguments for preprocessing all raw snapshot dataset files in a folder.

    Returns:
        argparse.Namespace: Parsed arguments input folder path, output folder path,
        delimiter, and the number of lines to skip.
    """
    parser = argparse.ArgumentParser(description="Run DatasetPreprocessing on all raw snapshot dataset files in a folder.")
    parser.add_argument('-i', '--input_folder', required=True, help='Input folder containing raw snapshot files')
    parser.add_argument('-o', '--output_folder', required=True, help='Output folder for preprocessed snapshot files')
    parser.add_argument('-d', '--delimiter', required=True, help='Delimiter to use')
    parser.add_argument('-s', '--skip', type=int, required=True, help='Number of header lines to skip')
    return parser.parse_args()

def main():
    args = parse_args()

    FILE_PREPROCESSING = "../../code/Tonic-build/DataPreprocessing"
    os.makedirs(args.output_folder, exist_ok=True)

    input_files = sorted([f for f in os.listdir(args.input_folder) if os.path.isfile(os.path.join(args.input_folder, f))])

    for filename in input_files:
        input_path = os.path.join(args.input_folder, filename)
        output_filename = f"preprocessed_{filename}"
        output_path = os.path.join(args.output_folder, output_filename)

        subprocess.run([
            FILE_PREPROCESSING,
            input_path,
            args.delimiter,
            str(args.skip),
            output_path
        ], check=True)

        print(f"Processed {input_path} -> {output_path}")

if __name__ == "__main__":
    main()
