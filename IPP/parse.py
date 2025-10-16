import sys
import argparse

import xml.etree.ElementTree as ET
from src.parser import Parser
from src.semantic_analyzer import SemanticAnalyzer


def get_input_parser():
    parser = argparse.ArgumentParser(description="SOL25 Parser")
    parser.add_argument("--source", type=str, help="Path to input file (optional, otherwise reads from stdin).")
    parser.add_argument("--output", type=str, help="Path to output file (optional, otherwise prints to stdout).")

    try: 
        args = parser.parse_args()
    except ValueError:
        exit(10)
    return args

def read_input(source=None):
    """Reads input from a file if given, otherwise from standard input."""
    if source:
        try:
            with open(source, "r", encoding="utf-8") as f:
                return f.read()
        except FileNotFoundError:
            print("Error: Input file not found.", file=sys.stderr)
            sys.exit(11) 
        except PermissionError:
            print("Error: Permission denied when accessing the file.", file=sys.stderr)
            sys.exit(11)
    else:
        return sys.stdin.read()

def write_output(output=None, data=None):
    """Writes output to a file if given, otherwise to standard output."""
    if output:
        try:
            with open(output, "w", encoding="utf-8") as f:
                f.write(data)
        except PermissionError:
            print("Error: Permission denied when accessing the file.", file=sys.stderr)
            sys.exit(12)
    else:
        print(data)


def generate_xml(ast, args):
    """Generates XML represantation of the Abstract Syntax Tree (AST and ) and writes it to the specified output.

    Args:
        ast: The AST object to be converted to XML.
        args: The command-line arguments containing the output file path.
    """
    root = ast.to_xml()
    tree = ET.ElementTree(root)
    ET.indent(tree, space="  ") 
    xml_declaration = '<?xml version="1.0" encoding="UTF-8"?>\n'
    xml_string = ET.tostring(root, encoding='unicode')
    
    write_output(args.output, xml_declaration + xml_string)


def main():
    # Parse input arguments
    args = get_input_parser()
    # Read input data
    input_data = read_input(args.source)
    if input_data == "":
        print("Error: Empty input.", file=sys.stderr)
        sys.exit(11)
    #Initialize parser + lexer
    parser = Parser()
    
    try:
        # Parse input data
        ast = parser.parse(input_data)
    except KeyError:
        print("Lexical error occurred.")
        sys.exit(21)
    except SyntaxError:
        print("Syntax error occurred.")
        sys.exit(22)
    # Initialize semantic analyzer
    analyzer = SemanticAnalyzer(ast)
    try:
        # Analyze the AST
        analyzer.analyze()
    except Exception as e:
        exit(e.args[0])
    # Generate XML output
    generate_xml(ast, args)

if __name__ == "__main__":
    try:
        main()  
    except Exception as e:
        print(e)
        sys.exit(99)
