"""
Gera o mapa de Althea de forma headless: chao + iluminacao + PlayerStarts +
a fatia da vila de Arakas (casas, poco central, muralha e torre de vigia).
Sem interacao de GUI. As estruturas sao primitivas do engine (cubos/cilindros)
escaladas — placeholders ate termos assets 3D.

Rodar via: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="...gen_test_level.py"
"""
import unreal

LEVEL_PATH = "/Game/Maps/L_Arakas_Test"

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

CUBE = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
CYLINDER = unreal.load_asset("/Engine/BasicShapes/Cylinder.Cylinder")


def spawn(cls, location, rotation=None):
    rot = rotation if rotation is not None else unreal.Rotator(0.0, 0.0, 0.0)
    return actors.spawn_actor_from_class(cls, unreal.Vector(*location), rot)


def structure(mesh, location, scale, label, yaw=0.0):
    """Spawna um StaticMeshActor (com colisao) escalado, como bloco de cenario."""
    a = spawn(unreal.StaticMeshActor, location, unreal.Rotator(0.0, 0.0, yaw))
    a.set_actor_label(label)
    a.static_mesh_component.set_static_mesh(mesh)
    a.set_actor_scale3d(unreal.Vector(*scale))
    return a


# Remove qualquer mapa anterior nesse caminho e cria um novo e vazio.
if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
    unreal.EditorAssetLibrary.delete_asset(LEVEL_PATH)
les.new_level(LEVEL_PATH)

# --- Chao 5000x5000 (cubo do engine escalado) ---
floor = spawn(unreal.StaticMeshActor, (0.0, 0.0, 0.0))
floor.set_actor_label("Floor")
floor.static_mesh_component.set_static_mesh(CUBE)
floor.set_actor_scale3d(unreal.Vector(50.0, 50.0, 1.0))

# --- Iluminacao ---
sun = spawn(unreal.DirectionalLight, (0.0, 0.0, 1000.0),
            unreal.Rotator(roll=0.0, pitch=-50.0, yaw=45.0))
sun.set_actor_label("Sun")

skylight = spawn(unreal.SkyLight, (0.0, 0.0, 600.0))
skylight.set_actor_label("SkyLight")

# --- Pontos de spawn (no centro da vila, acima do topo do chao z~120) ---
spawn(unreal.PlayerStart, (0.0, 300.0, 120.0)).set_actor_label("PlayerStart_1")
spawn(unreal.PlayerStart, (0.0, -300.0, 120.0)).set_actor_label("PlayerStart_2")

# =====================================================================
#  Fatia da vila de Arakas
#  Coordenadas evitam os PlayerStarts (0, +-300) e os pontos de spawn de
#  monstros (~+-1900) para nao prender ninguem nem bloquear o engajamento.
# =====================================================================

# --- Poco central da praca (cilindro baixo) ---
structure(CYLINDER, (0.0, 0.0, 50.0), (1.6, 1.6, 1.0), "Arakas_Poco")

# --- Casas ao redor da praca (blocos solidos com telhado sugerido pela altura) ---
houses = [
    (( 750.0,  750.0, 130.0), 25.0, "Casa_NE"),
    (( 750.0, -750.0, 130.0), -25.0, "Casa_SE"),
    ((-750.0,  750.0, 130.0), 200.0, "Casa_NO"),
    ((-750.0, -750.0, 130.0), 155.0, "Casa_SO"),
    (( 1150.0,  100.0, 130.0), 90.0, "Casa_L"),
    ((-1150.0, -150.0, 130.0), 90.0, "Casa_O"),
]
for loc, yaw, label in houses:
    # Corpo da casa (~360 x 300 x 260)
    structure(CUBE, loc, (3.6, 3.0, 2.6), label, yaw)
    # Telhado (bloco achatado e levemente maior, mais alto)
    roof_loc = (loc[0], loc[1], loc[2] + 150.0)
    structure(CUBE, roof_loc, (4.0, 3.4, 0.5), label + "_Telhado", yaw)

# --- Torre de vigia (cilindro alto num canto da vila) ---
structure(CYLINDER, (1400.0, 1400.0, 300.0), (2.2, 2.2, 6.0), "Arakas_Torre")
structure(CUBE, (1400.0, 1400.0, 640.0), (2.8, 2.8, 0.4), "Arakas_Torre_Topo")

# --- Muralha perimetral: 4 segmentos finos formando um quadrado com aberturas ---
# Cada lado tem ~2 segmentos com um vao no meio (portao).
WALL_H = 3.0          # ~300 uu de altura
WALL_T = 0.6          # espessura
EXTENT = 2050.0       # meia-largura do anel
SEG = 16.0            # comprimento de um segmento (~1600 uu)
GAP = 350.0           # meio-vao do portao

wall_segments = [
    # Lado Norte (+Y) e Sul (-Y): segmentos ao longo de X
    (( EXTENT - SEG * 50 + 800.0,  EXTENT, WALL_H * 50), (SEG, WALL_T, WALL_H), "Muro_N1"),
    ((-(EXTENT - SEG * 50 + 800.0), EXTENT, WALL_H * 50), (SEG, WALL_T, WALL_H), "Muro_N2"),
    (( EXTENT - SEG * 50 + 800.0, -EXTENT, WALL_H * 50), (SEG, WALL_T, WALL_H), "Muro_S1"),
    ((-(EXTENT - SEG * 50 + 800.0), -EXTENT, WALL_H * 50), (SEG, WALL_T, WALL_H), "Muro_S2"),
    # Lado Leste (+X) e Oeste (-X): segmentos ao longo de Y
    (( EXTENT,  EXTENT - SEG * 50 + 800.0, WALL_H * 50), (WALL_T, SEG, WALL_H), "Muro_L1"),
    (( EXTENT, -(EXTENT - SEG * 50 + 800.0), WALL_H * 50), (WALL_T, SEG, WALL_H), "Muro_L2"),
    ((-EXTENT,  EXTENT - SEG * 50 + 800.0, WALL_H * 50), (WALL_T, SEG, WALL_H), "Muro_O1"),
    ((-EXTENT, -(EXTENT - SEG * 50 + 800.0), WALL_H * 50), (WALL_T, SEG, WALL_H), "Muro_O2"),
]
for loc, scale, label in wall_segments:
    structure(CUBE, loc, scale, label)

# Salva o nivel.
les.save_current_level()
unreal.log("=== T4C: vila de Arakas gerada e salva em %s ===" % LEVEL_PATH)
