Prepare : PreAslToken

PreAslToken : 
	$(ECHO) Name(SYNI,0x$(SYNI:h=)) $(EOL) > $(SynTCGAslVar_DIR)\MyToken.asl
	$(ECHO) Name(SYNN,0x$(SYNN:h=)) $(EOL) >> $(SynTCGAslVar_DIR)\MyToken.asl
	$(ECHO) Name(TRST,0x$(_TRST_:h=)) $(EOL) >> $(SynTCGAslVar_DIR)\MyToken.asl 