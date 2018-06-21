
DefinitionBlock (
  "fTpmSsdt.aml",
  "SSDT",
  0x01,
  "Ssdt",
  "PttSsdt",
  0x1000
  )
{

  OperationRegion(FTNV,SystemMemory,0xFFFF0000,0xAA55)
  Field(FTNV,AnyAcc,Lock,Preserve)
  {
  CRBI, 64,     ///   (000) Intel PTT Allocated Buffer Address
  }
}