#!/usr/bin/env python3
import json
import os
import subprocess
import sys
from fnmatch import fnmatch

postprocess_rule_env = os.environ.copy()
postprocess_rule_env["SCRIPT_OUTPUT_FILE_COUNT"] = "1"
postprocess_rule_env["SCRIPT_OUTPUT_FILE_LIST_COUNT"] = "0"

excluded_patterns = os.environ.get("EXCLUDED_SOURCE_FILE_NAMES", "").split()
included_patterns = os.environ.get("INCLUDED_SOURCE_FILE_NAMES", "").split()

migrate_rule = os.environ["SCRIPT_INPUT_FILE_1"]
tapi_filelist = {"headers": [], "version": "3"}

for file_number in range(int(os.environ["SCRIPT_OUTPUT_FILE_LIST_COUNT"])):
    input_file_list = open(os.environ[f"SCRIPT_INPUT_FILE_LIST_{file_number}"])
    output_file_list = open(os.environ[f"SCRIPT_OUTPUT_FILE_LIST_{file_number}"])
    for input_file, output_file in zip(input_file_list, output_file_list):
        input_file = input_file.rstrip()
        output_file = output_file.rstrip()
        visibility = "private" if "PrivateHeaders/" in output_file else "public"

        input_name = os.path.basename(input_file)
        if input_name != os.path.basename(output_file):
            sys.exit(f"error: Trying to migrate {input_file} to {output_file}, but the file names don't match. "
                     "Headers should always have the same name when migrated. "
                     "Ensure that the paths in MigrateHeaders-input.xcfilelist match the order in MigrateHeaders-output.xcfilelist.")
        if any(fnmatch(input_name, pattern) or fnmatch(input_file, pattern) for pattern in excluded_patterns) \
                and not any(fnmatch(input_name, pattern) or fnmatch(input_file, pattern) for pattern in included_patterns):
            continue

        tapi_filelist["headers"].append({
            "path": output_file,
            "type": visibility
        })

        postprocess_rule_env["INPUT_FILE_PATH"] = input_file
        postprocess_rule_env["INPUT_FILE_NAME"] = input_name
        postprocess_rule_env["SCRIPT_OUTPUT_FILE_0"] = output_file
        postprocess_rule_env["SCRIPT_HEADER_VISIBILITY"] = visibility
        print("RuleScriptExecution", output_file, input_file, file=sys.stderr)
        subprocess.check_call((migrate_rule), env=postprocess_rule_env)

tapi_filelist_path = os.environ.get("SCRIPT_OUTPUT_FILE_1")
if tapi_filelist_path:
    with open(tapi_filelist_path, "w") as fd:
        json.dump(tapi_filelist, fd, indent=2)
