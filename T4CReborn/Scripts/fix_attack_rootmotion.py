"""
Desliga o Root Motion das animações de ataque do Mannequin. Elas vêm com root
motion ligado, o que quebra a pose/desloca o personagem ao tocá-las como montage
de slot (o combate já trata posição/dano). In-place fica limpo.

Rodar via: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript="...fix_attack_rootmotion.py"
"""
import unreal

ATTACKS = [
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01",
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02",
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_03",
    "/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_ChargedAttack",
]

for path in ATTACKS:
    a = unreal.load_asset(path)
    if not a:
        unreal.log_warning("nao carregou: %s" % path)
        continue
    a.set_editor_property("enable_root_motion", False)
    unreal.EditorAssetLibrary.save_loaded_asset(a)
    unreal.log("=== T4C: root motion OFF em %s ===" % path)
