"""
Gera o material base de cor do T4C (headless), pois o BasicShapeMaterial da engine
não expõe um parâmetro de cor utilizável em runtime. Cria /Game/Materials/M_T4C_Colored
com um VectorParameter "Color" ligado ao BaseColor (+ leve Emissive p/ destacar).
O código cria um MaterialInstanceDynamic desse material e aplica a cor por ator.

Rodar via: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="...gen_materials.py"
"""
import unreal

PKG = "/Game/Materials"
NAME = "M_T4C_Colored"
PATH = PKG + "/" + NAME

mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()

if unreal.EditorAssetLibrary.does_asset_exist(PATH):
    unreal.EditorAssetLibrary.delete_asset(PATH)

mat = tools.create_asset(NAME, PKG, unreal.Material, unreal.MaterialFactoryNew())

# VectorParameter "Color" -> Base Color
color = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -500, 0)
color.set_editor_property("parameter_name", "Color")
color.set_editor_property("default_value", unreal.LinearColor(0.8, 0.8, 0.8, 1.0))
mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)

# Emissive = Color * 0.18 (leve brilho p/ leitura)
mult = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -250, 220)
scale = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -500, 260)
scale.set_editor_property("r", 0.18)
mel.connect_material_expressions(color, "", mult, "A")
mel.connect_material_expressions(scale, "", mult, "B")
mel.connect_material_property(mult, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

mel.recompile_material(mat)
unreal.EditorAssetLibrary.save_asset(PATH)
unreal.log("=== T4C: material %s gerado e salvo ===" % PATH)
