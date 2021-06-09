typedef 
BOOLEAN
(* KERNEL_IMAGE_ENTRY_POINT) (
    IN UINTN                 NoEntries,
    IN EFI_MEMORY_DESCRIPTOR *MemoryMap,
    IN UINTN                 MapKey,
    IN VOID                  *SALProc,
    IN VOID                  *PALProc,
    IN VOID                  *EfiGP
	);


