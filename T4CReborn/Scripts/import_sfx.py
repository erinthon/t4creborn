"""
Importa os .wav de Saved/SFX_src para /Game/Audio como SoundWaves.
Rodar via: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="...import_sfx.py"
(Gerar os .wav antes com Scripts/gen_sfx.py)
"""
import unreal
import os

SRC = r"C:/dev/t4c/T4CReborn/Saved/SFX_src"
DEST = "/Game/Audio"
FILES = ["sfx_swing.wav", "sfx_hit.wav", "sfx_levelup.wav", "sfx_death.wav", "sfx_monster_death.wav"]

tools = unreal.AssetToolsHelpers.get_asset_tools()
tasks = []
for f in FILES:
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", os.path.join(SRC, f))
    t.set_editor_property("destination_path", DEST)
    t.set_editor_property("automated", True)
    t.set_editor_property("save", True)
    t.set_editor_property("replace_existing", True)
    tasks.append(t)

tools.import_asset_tasks(tasks)

for f in FILES:
    asset_path = "%s/%s" % (DEST, os.path.splitext(f)[0])
    ok = unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    unreal.log("=== T4C SFX import: %s -> %s ===" % (asset_path, "OK" if ok else "FALHOU"))
