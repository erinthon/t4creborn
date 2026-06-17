"""
Importa o Stylized Goblin (FBX skeletal + texturas PBR) e monta um material.
Saída: /Game/Monsters/Goblin (SK_Goblin) + /Game/Monsters/Goblin/Textures + M_Goblin.
Rodar via: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="...import_goblin.py"
"""
import unreal
import os

SRC = r"C:/dev/t4c/T4CReborn/Saved/goblin_pack/Stylized Goblin"
DEST = "/Game/Monsters/Goblin"
TEX_DEST = DEST + "/Textures"
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary

# --- 1) Importa o FBX como skeletal mesh ---
fbx_task = unreal.AssetImportTask()
fbx_task.set_editor_property("filename", os.path.join(SRC, "SK_Goblin.fbx"))
fbx_task.set_editor_property("destination_path", DEST)
fbx_task.set_editor_property("automated", True)
fbx_task.set_editor_property("save", True)
fbx_task.set_editor_property("replace_existing", True)
opts = unreal.FbxImportUI()
opts.set_editor_property("import_mesh", True)
opts.set_editor_property("import_as_skeletal", True)
opts.set_editor_property("import_materials", False)
opts.set_editor_property("import_textures", False)
opts.set_editor_property("import_animations", False)
fbx_task.set_editor_property("options", opts)
tools.import_asset_tasks([fbx_task])

# --- 2) Importa as texturas ---
TEX = {
    "BaseColor": "goblin base_DefaultMaterial_BaseColor.png",
    "Normal":    "goblin base_DefaultMaterial_Normal.png",
    "Metallic":  "goblin base_DefaultMaterial_Metallic.png",
    "Roughness": "goblin base_DefaultMaterial_Roughness.png",
}
tex_tasks = []
for key, fname in TEX.items():
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", os.path.join(SRC, "Textures", fname))
    t.set_editor_property("destination_path", TEX_DEST)
    t.set_editor_property("automated", True)
    t.set_editor_property("save", True)
    t.set_editor_property("replace_existing", True)
    tex_tasks.append((key, t))
tools.import_asset_tasks([t for _, t in tex_tasks])


def load_tex(key, fname):
    # A UE sanitiza o nome do asset (troca espaço por underscore) ao importar.
    asset_name = os.path.splitext(fname)[0].replace(" ", "_")
    p = "%s/%s" % (TEX_DEST, asset_name)
    return unreal.load_asset(p)


textures = {k: load_tex(k, f) for k, f in TEX.items()}
for k, tx in textures.items():
    unreal.log("=== T4C Goblin tex %s: %s ===" % (k, "OK" if tx else "NAO ACHOU"))

# Ajusta sRGB/compressão (Normal/Metallic/Roughness são lineares).
for key in ("Normal", "Metallic", "Roughness"):
    tx = textures.get(key)
    if tx:
        tx.set_editor_property("srgb", False)
if textures.get("Normal"):
    textures["Normal"].set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
for key in ("Normal", "Metallic", "Roughness", "BaseColor"):
    if textures.get(key):
        unreal.EditorAssetLibrary.save_loaded_asset(textures[key])

# --- 3) Cria o material M_Goblin ---
mat_path = DEST + "/M_Goblin"
if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
    unreal.EditorAssetLibrary.delete_asset(mat_path)
mat = tools.create_asset("M_Goblin", DEST, unreal.Material, unreal.MaterialFactoryNew())


def add_sample(tex, x, y, sampler):
    s = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, x, y)
    s.set_editor_property("texture", tex)
    s.set_editor_property("sampler_type", sampler)
    return s

ST = unreal.MaterialSamplerType
if textures.get("BaseColor"):
    s = add_sample(textures["BaseColor"], -400, -200, ST.SAMPLERTYPE_COLOR)
    mel.connect_material_property(s, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
if textures.get("Normal"):
    s = add_sample(textures["Normal"], -400, 100, ST.SAMPLERTYPE_NORMAL)
    mel.connect_material_property(s, "RGB", unreal.MaterialProperty.MP_NORMAL)
if textures.get("Metallic"):
    s = add_sample(textures["Metallic"], -400, 400, ST.SAMPLERTYPE_LINEAR_GRAYSCALE)
    mel.connect_material_property(s, "R", unreal.MaterialProperty.MP_METALLIC)
if textures.get("Roughness"):
    s = add_sample(textures["Roughness"], -400, 700, ST.SAMPLERTYPE_LINEAR_GRAYSCALE)
    mel.connect_material_property(s, "R", unreal.MaterialProperty.MP_ROUGHNESS)

# Necessário para aplicar o material a uma malha esqueletal sem aviso.
mat.set_editor_property("used_with_skeletal_mesh", True)
mel.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat)

skm_exists = unreal.EditorAssetLibrary.does_asset_exist(DEST + "/SK_Goblin")
unreal.log("=== T4C Goblin: SK_Goblin=%s, M_Goblin criado ===" % ("OK" if skm_exists else "FALHOU"))
