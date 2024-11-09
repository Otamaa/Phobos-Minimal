
.586
.model flat, C
option casemap :none
option prologue:none
option epilogue:none
option language: basic ; invalid language removes leading "_" from output name.


;
;  This file mostly contains the constructors what we need to jump to
;  without any additional code being rolled out.
;

.code

;
;  Implementation macro.
;
ASM_DEFINE_IMPLEMENTATION macro name, address
    name proc
        mov eax, address
        jmp eax
        ; ECHO Warning: MakeName("0x&address", "&name")
    name endp
    align 10h
endm

;================================================================================================================
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0FieldClass@@QAE@PADJ@Z 004CB700h
;ASM_DEFINE_IMPLEMENTATION ??0FieldClass@@QAE@PADK@Z 004CB760h
;ASM_DEFINE_IMPLEMENTATION ??0FieldClass@@QAE@PAD0@Z 004CB7C0h
;ASM_DEFINE_IMPLEMENTATION ??0FieldClass@@QAE@PADPAXH@Z 004CB830h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0EventClass@@QAE@HW4EventType@@W4AbstractType@@H_N@Z 004C6970h
;ASM_DEFINE_IMPLEMENTATION ??0EventClass@@QAE@HW4EventType@@W4AbstractType@@HHABVCellStruct@@@Z 004C6AE0h
;ASM_DEFINE_IMPLEMENTATION ??0EventClass@@QAE@HW4EventType@@@Z 004C66C0h
;ASM_DEFINE_IMPLEMENTATION ??0EventClass@@QAE@HW4EventType@@HABVCellStruct@@@Z 004C6B60h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0ConvertClass@@QAE@PBUBytePalette@@0PAVDSurface@@I_N@Z 0048E740h
;ASM_DEFINE_IMPLEMENTATION ??0ConvertClass@@QAE@ABUBytePalette@@0PAVDSurface@@I_N@Z 0048E740h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0ControlClass@@QAE@IHHHHW4GadgetFlag@@_N@Z 0048E520h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0UDPInterfaceClass@@QAE@XZ 007B2DB0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0LoadOptionsClass@@QAE@XZ 00558740h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0TeamClass@@QAE@PAVTeamTypeClass@@PAVHouseClass@@H@Z 006E8A90h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0OverlayClass@@QAE@PAVOverlayTypeClass@@ABVCellStruct@@H@Z 005FC380h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0AirstrikeClass@@QAE@PAVTechnoClass@@@Z 0041D380h
;ASM_DEFINE_IMPLEMENTATION ??0AirstrikeClass@@QAE@XZ 0041D300h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0SpawnManagerClass@@QAE@PAVTechnoClass@@PAVAircraftTypeClass@@HHH@Z 006B6C90h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0SlaveManagerClass@@QAE@PAVTechnoClass@@PAVInfantryTypeClass@@HHH@Z 006AF1A0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0ParticleClass@@QAE@PAVParticleTypeClass@@ABUCoordStruct@@1PAVParticleSystemClass@@@Z 0062B5E0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0LineTrail@@QAE@XZ 00556A20h
;ASM_DEFINE_IMPLEMENTATION ??1LineTrail@@QAE@XZ 00556B30h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0CaptureManagerClass@@QAE@PAVTechnoClass@@H_N@Z 004717D0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0AlphaShapeClass@@QAE@PAVObjectClass@@HH@Z 00420960h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0TemporalClass@@QAE@PAVTechnoClass@@@Z 0071A4E0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0BuildingLightClass@@QAE@PAVObjectClass@@@Z 00435820h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0ParasiteClass@@QAE@PAVFootClass@@@Z 006292B0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0TargetClass@@QAE@PAVAbstractClass@@@Z 006E6AB0h
;ASM_DEFINE_IMPLEMENTATION ??0TargetClass@@QAE@PBVCellStruct@@@Z 006E6B20h
;ASM_DEFINE_IMPLEMENTATION ??0TargetClass@@QAE@PBVCoordStruct@@@Z 006E6B70h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0VoxClass@@QAE@PAD@Z 00752CB0h
;ASM_DEFINE_IMPLEMENTATION ??1VoxClass@@QAE@XZ 00752D60h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0WaveClass@@QAE@ABUCoordStruct@@0PAVTechnoClass@@W4WaveType@@PAVAbstractClass@@@Z 0075E950h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0BulletClass@@QAE@XZ 00466380h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0EMPulseClass@@QAE@VCellStruct@@HHPAVTechnoClass@@@Z 004C52B0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0IonBlastClass@@QAE@UCoordStruct@@@Z 0053CB10h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0ScriptClass@@QAE@PAVScriptTypeClass@@@Z 006913C0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0CCFileClass@@QAE@PBD@Z 004739F0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0LightSourceClass@@QAE@UCoordStruct@@HHUTintStruct@@@Z 00554760h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0RadSiteClass@@QAE@XZ 0065B1E0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0SpotlightClass@@QAE@UCoordStruct@@H@Z 005FF250h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0LightConvertClass@@QAE@PAUBytePalette@@0PAVSurface@@HHH_NPAEI@Z 00555DA0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0LaserDrawClass@@QAE@UCoordStruct@@0HEUColorStruct@@11H_N2MM@Z 0054FE60h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0VoxelAnimClass@@QAE@PAVVoxelAnimTypeClass@@PAUCoordStruct@@PAVHouseClass@@@Z 007493B0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0ParticleSystemClass@@QAE@PAVParticleSystemTypeClass@@ABUCoordStruct@@PAVAbstractClass@@PAVTechnoClass@@1PAVHouseClass@@@Z 0062DC50h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ?sin@Math@@YGNM@Z 004CB150h
;ASM_DEFINE_IMPLEMENTATION ?cos@Math@@YGNM@Z 004CB1A0h
;ASM_DEFINE_IMPLEMENTATION ?asin@Math@@YGNM@Z 004CB260h
;ASM_DEFINE_IMPLEMENTATION ?tan@Math@@YGNM@Z 004CB320h
;ASM_DEFINE_IMPLEMENTATION ?acos@Math@@YGNM@Z 004CB290h
;ASM_DEFINE_IMPLEMENTATION ?atan@Math@@YGNM@Z 004CB480h

;ASM_DEFINE_IMPLEMENTATION ?sin@Math@@YAMN@Z 004CACB0h
;ASM_DEFINE_IMPLEMENTATION ?cos@Math@@YAMN@Z 004CAD00h
;ASM_DEFINE_IMPLEMENTATION ?sqrt@Math@@YAMN@Z 004CAC40h

;ASM_DEFINE_IMPLEMENTATION ?atan2@Math@@YAMNN@Z 004CAE30h
;ASM_DEFINE_IMPLEMENTATION ?arctanfoo@Math@@YAMNN@Z 004CAE30h

;ASM_DEFINE_IMPLEMENTATION ?tan@Math@@YANN@Z 004CAD50h
;ASM_DEFINE_IMPLEMENTATION ?asin@Math@@YANN@Z 004CAD80h
;ASM_DEFINE_IMPLEMENTATION ?acos@Math@@YANN@Z 004CADB0h
;ASM_DEFINE_IMPLEMENTATION ?atan@Math@@YANN@Z 004CADE0h

;ASM_DEFINE_IMPLEMENTATION ?atan2@Math@@YGNMM@Z 004CB3D0h

;ASM_DEFINE_IMPLEMENTATION ?sqrt@Math@@YGMM@Z 004CB060h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0VeinholeMonsterClass@@QAE@PAVCellStruct@@@Z 0074C5B0h
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ?Allocate@@YAPAXI@Z 007C8E17h
;ASM_DEFINE_IMPLEMENTATION ?Deallocate@@YAXPBX@Z 007C8B3Dh
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1HouseTypeClass@@UAE@XZ 00512760h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1HouseClass@@UAE@XZ 0050E380h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1DropshipStruct@@UAE@XZ 004B69D0h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0DropshipStruct@@QAE@XZ 004B69B0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0BaseClass@@QAE@XZ 0042E6F0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0UnitTrackerClass@@QAE@XZ 00748FD0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1TextLabelClass@@UAE@XZ 0072A670h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1FootClass@@UAE@XZ 004E0170h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0FoggedObjectClass@@QAE@XZ 004D08B0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0FactoryClass@@QAE@XZ 004C98B0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1EMPulseClass@@UAE@XZ 004C5AC0h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0EMPulseClass@@QAE@XZ 004C5370h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1EBolt@@UAE@XZ 004C2C10h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0EBolt@@QAE@XZ 004C1E10h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1ABuffer@@UAE@XZ 00410E50h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0ABuffer@@QAE@XZ 00410CE0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1ZBuffer@@UAE@XZ 007BCAE0h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0ZBuffer@@QAE@XZ 007BC970h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1DisplayClass@@UAE@XZ 004AEBF0h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0DisplayClass@@QAE@XZ 004A8730h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0DiskLaserClass@@QAE@XZ 004A7A30h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1CStreamClass@@UAE@XZ 004A2880h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0CStreamClass@@QAE@XZ 004A2820h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1ConvertClass@@UAE@XZ 00491210h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1ColorScheme@@UAE@XZ 0068C8D0h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0ColorScheme@@QAE@XZ 0068C650h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0PixelFXClassClass@@QAE@XZ 00631E10h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0CDDriveManagerClass@@QAE@XZ 004E6070h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1CCToolTip@@UAE@XZ 007784A0h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0INIClass@@QAE@XZ 00535AA0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1RAMFileClass@@UAE@XZ 0065C2A0h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1CDFileClass@@UAE@XZ 00535A60h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0CDFileClass@@QAE@XZ 0047AA30h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1RawFileClass@@UAE@XZ 0065CA00h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0BufferIOFileClass@@QAE@XZ 00431B20h ;ctor
;ASM_DEFINE_IMPLEMENTATION ??1BufferIOFileClass@@UAE@XZ 00431B80h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??0CCFileClass@@QAE@XZ 00473A80h ;ctor
;ASM_DEFINE_IMPLEMENTATION ??1CCFileClass@@UAE@XZ 004739F0h ;dtor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1AudioIDXData@@UAE@XZ 00401580h ;dtor
;================================================================================================================
ASM_DEFINE_IMPLEMENTATION ??1AnimClass@@UAE@XZ 00426590h ;dtor
ASM_DEFINE_IMPLEMENTATION ??0AnimClass@@QAE@XZ 00422720h ;ctor
ASM_DEFINE_IMPLEMENTATION ??0AnimClass@@QAE@PAVAnimTypeClass@@ABUCoordStruct@@HHW4AnimFlag@@H_N@Z 00421EA0h ;ctor
ASM_DEFINE_IMPLEMENTATION ??0AnimClass@@QAE@PAVAnimTypeClass@@ABUCoordStruct@@HHKH_N@Z 00421EA0h ;ctor
ASM_DEFINE_IMPLEMENTATION ??0AnimClass@@QAE@PAVAnimTypeClass@@PAUCoordStruct@@HHW4AnimFlag@@H_N@Z 00421EA0h ;ctor
;================================================================================================================
;ASM_DEFINE_IMPLEMENTATION ??1CellClass@@UAE@XZ 00487E80h ;dtor
;ASM_DEFINE_IMPLEMENTATION ??0CellClass@@QAE@XZ 0047BBF0h ;ctor
;================================================================================================================
end