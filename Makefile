.PHONY: clean All

All:
	@echo "----------Building project:[ Server - Debug ]----------"
	@cd "Server" && "$(MAKE)" -f  "Server.mk"
clean:
	@echo "----------Cleaning project:[ Server - Debug ]----------"
	@cd "Server" && "$(MAKE)" -f  "Server.mk" clean
