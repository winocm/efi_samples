typedef 
BOOLEAN
(* KERNEL_IMAGE_ENTRY_POINT) (
    IN UINTN    NoEntries,
    IN UINTN    MapKey,
    IN UINTN    DescriptorSize,
    IN UINT32   DescriptorVersion
	);


