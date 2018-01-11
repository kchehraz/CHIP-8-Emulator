class Chip8 {
public:
	Chip8();
	~Chip8();

	uint16_t opcode;        // Opcode
	uint16_t pc;            // Program counter
	uint16_t I;             // Index register

	uint16_t stack[16];     // Stack
	uint16_t sp;            // Stack pointer

	uint8_t memory[4096];   // Memory
	uint8_t V[16];          // Registers

	uint8_t gfx[64 * 32];   // Graphics buffer

	uint8_t delay_timer;
	uint8_t sound_timer;

	uint8_t key[16];        // Key state (0x0-0xF)

	bool drawFlag;

	void initialize();
	bool load(const char* filename);
	void emulateCycle();
};