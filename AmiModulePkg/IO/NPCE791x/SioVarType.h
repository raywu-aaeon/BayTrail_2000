#if NPCE791x_SERIAL_PORT1_PRESENT
  typedef struct {
 UINT8        DevImplemented;
 UINT16       DevBase1;
 UINT16       DevBase2;
 UINT8        DevIrq1;
 UINT8        DevIrq2;
 UINT8        DevDma1;
 UINT8        DevDma2;
 }  COMA_V_DATA;
 typedef struct {
 UINT8        DevEnable;
 UINT8        DevPrsId;
 UINT8        DevMode;
 }  COMA_NV_DATA;
#endif

#if NPCE791x_CIR_PORT_PRESENT
 typedef struct {
UINT8        DevImplemented;
UINT16       DevBase1;
UINT16       DevBase2;
UINT8        DevIrq1;
UINT8        DevIrq2;
UINT8        DevDma1;
UINT8        DevDma2;
}  CIR_V_DATA;
typedef struct {
UINT8        DevEnable;
UINT8        DevPrsId;
UINT8        DevMode;
}  CIR_NV_DATA;
#endif
