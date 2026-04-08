import os
import re
import sys
import json
import subprocess

# --- Naming Conversion Functions ---
def to_snake_case(name):
    if not name: return name
    is_member = name.startswith('_')
    inner = name[1:] if is_member else name
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', inner)
    res = re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
    return ('_' + res) if is_member else res

def to_pascal_case(name):
    if not name: return name
    components = name.split('_')
    return "".join(x.title() for x in components if x)

def to_camel_case(name):
    if not name: return name
    components = [x for x in name.split('_') if x]
    if not components: return name
    return components[0].lower() + "".join(x.title() for x in components[1:])

# --- Refactoring Logic ---
class KathrynRefactor:
    def __init__(self, src_root):
        self.src_root = src_root
        self.compile_commands = self.load_compile_commands()

    def load_compile_commands(self):
        # Look for compile_commands.json to get accurate info
        for root, dirs, files in os.walk('.'):
            if 'compile_commands.json' in files:
                with open(os.path.join(root, 'compile_commands.json'), 'r') as f:
                    return json.load(f)
        return []

    def fix_comments(self, content):
        # Rule 7: no //////// comment
        content = re.sub(r'/{5,}', '///', content)
        # Rule 8: comment should have only ///
        # Avoid changing /// already present, also handle #include and http://
        content = re.sub(r'(?<![:/"])\/\/(?!\/|\s*\/)', '///', content)
        # Rule 6: multiple line comment should be /***/
        def repl_multiline(match):
            text = match.group(0)
            if '\n' in text:
                return text
            return text
        content = re.compile(r'/\*.*?\*/', re.DOTALL).sub(repl_multiline, content)
        return content

    def identify_ptr_methods(self, file_path):
        """
        Identify methods that return a pointer.
        Uses grep to find possible pointer returns in the file.
        """
        results = []
        try:
            output = subprocess.check_output(['grep', '-nE', r'\w+\s*\*\s+\w+\s*\(', file_path], text=True)
            for line in output.splitlines():
                match = re.search(r'(\d+):(\w+)\s*\*\s+(\w+)\s*\(', line)
                if match:
                    results.append({'line': int(match.group(1)), 'name': match.group(3)})
        except subprocess.CalledProcessError:
            pass
        return results

    def refactor_file(self, file_path):
        with open(file_path, 'r') as f:
            content = f.read()

        # Apply comment rules first
        content = self.fix_comments(content)
        
        # Suffix pointers with _ptr
        ptr_methods = self.identify_ptr_methods(file_path)
        ptr_names = {m['name'] for m in ptr_methods}
        for m_name in ptr_names:
            if not m_name.endswith('_ptr'):
                content = re.sub(rf'\b{m_name}\b', f"{m_name}_ptr", content)
        
        return content

    def run(self):
        for root, dirs, files in os.walk(self.src_root):
            for file in files:
                if file.endswith(('.h', '.cpp', '.hpp')):
                    file_path = os.path.join(root, file)
                    print(f"Processing {file_path}...")
                    new_content = self.refactor_file(file_path)
                    with open(file_path, 'w') as f:
                        f.write(new_content)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 kathryn_refactor.py <src_folder>")
    else:
        refactor = KathrynRefactor(sys.argv[1])
        refactor.run()
