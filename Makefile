.PHONY:all clean
INC = /inc

all:
	@echo "Building client..." 
	@make -I$(INC) -C client/
	@echo "Building module..." 
	@make -I$(INC) -C module/

clean:
	@echo "Cleaning client..."
	@make -C client/ clean
	@echo "Cleaning module..."	
	@make -C module/ clean
