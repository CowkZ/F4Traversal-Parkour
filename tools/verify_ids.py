import sys
import struct

def verify_ids(bin_path, ids_to_check):
    try:
        with open(bin_path, 'rb') as f:
            data = f.read()
    except FileNotFoundError:
        print(f"Erro: Arquivo {bin_path} não encontrado.")
        return

    print(f"Analisando {bin_path}...")
    print("-" * 40)
    
    for target_id, name in ids_to_check.items():
        id_bytes = struct.pack('<I', target_id)
        if id_bytes in data:
            print(f"ID {target_id:7} ({name:25}) : [ OK ] Encontrado")
        else:
            print(f"ID {target_id:7} ({name:25}) : [ FAIL ] NÃO encontrado")

if __name__ == "__main__":
    IDS = {
        2230668: "bhkPickData ctor",
        2236622: "bhkPickData::SetStartEnd",
        2200263: "TESObjectCELL::Pick",
        2277770: "bhkPickData::HasHit",
        2277771: "bhkPickData::GetHitFraction",
        2228956: "Main::WorldRootCamera",
        2270344: "NiCamera::WorldPtToScreenPt3",
    }
    
    if len(sys.argv) < 2:
        print("Uso: python3 verify_ids.py <caminho_para_o_bin>")
    else:
        verify_ids(sys.argv[1], IDS)
