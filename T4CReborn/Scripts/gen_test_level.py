"""
Gera o mapa de teste de Althea (Fase 1) de forma headless.
Chão + iluminacao + 2 PlayerStarts. Sem interacao de GUI.
Rodar via: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="...gen_test_level.py"
"""
import unreal

LEVEL_PATH = "/Game/Maps/L_Arakas_Test"

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def spawn(cls, location, rotation=None):
    rot = rotation if rotation is not None else unreal.Rotator(0.0, 0.0, 0.0)
    return actors.spawn_actor_from_class(cls, unreal.Vector(*location), rot)


# Remove qualquer mapa anterior nesse caminho e cria um novo e vazio.
if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
    unreal.EditorAssetLibrary.delete_asset(LEVEL_PATH)
les.new_level(LEVEL_PATH)

# --- Chao 5000x5000 (cubo do engine escalado) ---
cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
floor = spawn(unreal.StaticMeshActor, (0.0, 0.0, 0.0))
floor.set_actor_label("Floor")
floor.static_mesh_component.set_static_mesh(cube)
floor.set_actor_scale3d(unreal.Vector(50.0, 50.0, 1.0))

# --- Iluminacao ---
sun = spawn(unreal.DirectionalLight, (0.0, 0.0, 1000.0),
            unreal.Rotator(roll=0.0, pitch=-50.0, yaw=45.0))
sun.set_actor_label("Sun")

skylight = spawn(unreal.SkyLight, (0.0, 0.0, 600.0))
skylight.set_actor_label("SkyLight")

# --- Pontos de spawn (acima do topo do chao, z~120) ---
spawn(unreal.PlayerStart, (0.0, 300.0, 120.0)).set_actor_label("PlayerStart_1")
spawn(unreal.PlayerStart, (0.0, -300.0, 120.0)).set_actor_label("PlayerStart_2")

# Salva o nivel.
les.save_current_level()
unreal.log("=== T4C: mapa de teste gerado e salvo em %s ===" % LEVEL_PATH)
