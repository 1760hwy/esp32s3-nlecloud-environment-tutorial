"""Check files Git would publish. Python 3.10+, standard library only.

No network or firmware changes. This catches known mistakes, not every secret.
Run from any directory; an initialized Git repository is required.
"""
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]


def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])


try:
    paths = sorted(set(git('ls-files', '-z', '--cached', '--others', '--exclude-standard')
                       .decode('utf-8').rstrip('\0').split('\0')) - {''})
except subprocess.CalledProcessError:
    raise SystemExit('Run git init first; the checker inspects the Git publication boundary.')

errors = []
required = ['README.md', 'README_CN.md', 'CMakeLists.txt', 'sdkconfig.defaults',
            'dependencies.lock', 'main/idf_component.yml', 'main/app_secrets.example.h',
            'SECURITY.md', 'docs/VALIDATION.md', 'docs/THIRD_PARTY.md']
for name in required:
    if name not in paths:
        errors.append(f'Missing public file: {name}')

blocked_parts = {'build', 'managed_components', '.vscode', '.idea', '__pycache__'}
blocked_suffixes = {'.bin', '.elf', '.map', '.pcap', '.pcapng', '.pem', '.key', '.pfx', '.p12', '.log'}
secret_patterns = [
    r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----',
    r'\bgh[pousr]_[A-Za-z0-9]{20,}\b',
    r'\bgithub_pat_[A-Za-z0-9_]{20,}\b',
    r'\bAKIA[A-Z0-9]{16}\b',
    r'://[^\s/]+:[^\s/@]+@',
    r'\b[0-9a-fA-F]{32}\b',  # Device SecretKeys commonly have this form.
]


def heading_ids(text):
    counts = {}
    anchors = set()
    in_fence = False
    for line in text.splitlines():
        if line.startswith('```'):
            in_fence = not in_fence
        if in_fence or not re.match(r'^#{1,6}\s+', line):
            continue
        title = re.sub(r'^#{1,6}\s+', '', line).strip().lower()
        title = re.sub(r'[^\w\- ]', '', title, flags=re.UNICODE).replace(' ', '-')
        n = counts.get(title, 0)
        counts[title] = n + 1
        anchors.add(title if n == 0 else f'{title}-{n}')
    return anchors


for name in paths:
    p = Path(name)
    if (set(p.parts) & blocked_parts or any(x.startswith('build-') for x in p.parts)
            or p.name in {'app_secrets.h', 'sdkconfig', 'sdkconfig.old'}
            or p.name.startswith('.env') or p.suffix.lower() in blocked_suffixes):
        errors.append(f'Private/generated file in publication set: {name}')
    file = ROOT / name
    if not file.is_file():
        errors.append(f'File not on disk: {name}')
        continue
    if file.suffix.lower() not in {'.md', '.c', '.h', '.txt', '.py', '.yml', '.svg', '.lock', ''}:
        continue
    text = file.read_text(encoding='utf-8-sig')
    # Published license copies contain authors, not credentials; retain notices.
    for pattern in secret_patterns:
        if re.search(pattern, text):
            errors.append(f'Possible credential pattern in {name}; inspect privately')
            break
    if p.suffix == '.md':
        for target in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)', text):
            if re.match(r'^[a-z]+:', target, re.I):
                continue
            rel, _, anchor = unquote(target).partition('#')
            resolved = (file.parent / rel).resolve() if rel else file
            if not resolved.is_relative_to(ROOT) or not resolved.is_file():
                errors.append(f'Broken/outside local link in {name}: {target}')
            elif anchor and resolved.suffix == '.md':
                if anchor not in heading_ids(resolved.read_text(encoding='utf-8-sig')):
                    errors.append(f'Broken heading link in {name}: {target}')

for name in ['main/app_secrets.h', 'sdkconfig', 'build/example.bin']:
    result = subprocess.run(['git', '-C', str(ROOT), 'check-ignore', '-q', '--no-index', name])
    if result.returncode != 0:
        errors.append(f'Missing ignore rule: {name}')

template = (ROOT / 'main/app_secrets.example.h').read_text(encoding='utf-8')
for macro, value in re.findall(r'^#define\s+(APP_\w+)\s+"([^"]*)"', template, re.M):
    if macro != 'APP_MQTT_URI' and not value.startswith('YOUR_'):
        errors.append(f'Non-placeholder template value: {macro}')

if errors:
    print('\n'.join(errors))
    sys.exit(1)
print(f'PASS: {len(paths)} publishable files; private/build paths excluded; '
      'known secret patterns, local links, heading anchors and template checked.')
print('This is a static publication check, not a hardware/cloud acceptance test.')
