TEST_01_ROM = (
	"01.basics.nes",
	b"51819e8e502bd88fe3b7244198a074dbeef2e848f66c587be04b04f1f0d4bb52"
)
TEST_01_BIN = (
	"01.bin",
	b"83d15be3a3ae1d718872921e2135c7db1034059ae803aaa0fdc0ad075f233b55"
)

TEST_02_ROM = (
	"02.alignment.nes",
	b"125bbb3ce1e67370f1f4559c2ad3221e52a3e98880b9789400292b5f3a8b39e6"
)
TEST_02_BIN = (
	"02.bin",
	b"57dc5946584144ceb4cc00f64bd5acdae204307bd025c3cb80954410c9e9f1de"
)

TEST_03_ROM = (
	"03.corners.nes",
	b"9dd57776bc6267fe6183c5521d67cbe3fccc6662ae545eb2c419949bf39644d3"
)
TEST_03_BIN = (
	"03.bin",
	b"bd7519add80c0f7d1989c6ca5d6f0945f1a51c3506c45a7c717e2e1a0cddb82a"
)

TEST_04_ROM = (
	"04.flip.nes",
	b"5f7142bddb51b7577f93fa22f9f668efebbeea00346d7255089e1863acb9d46a"
)
TEST_04_BIN = (
	"04.bin",
	b"46d848fdcb3bdca3172ffd1ab274a736af380531d4429dde9fa6e09265036bb0"
)

TEST_05_ROM = (
	"05.left_clip.nes",
	b"69b329658c17b953f149c2f0de77eb272089df22c815bd2fd3d6f43206791c13"
)
TEST_05_BIN = (
	"05.bin",
	b"ecfa9b624d3eb50dd46e943202a86465353f2a771a1b53b9ae43431018f6cf60"
)

TEST_06_ROM = (
	"06.right_edge.nes",
	b"8e6653fcb869e06873e29e5e4423122ea72ba0bf38f3ba9e39f471420db759a4"
)

TEST_07_ROM = (
	"07.screen_bottom.nes",
	b"05849956f80267838c5b6556310266b794078a4300841cbb36339fd141905a0b"
)

TEST_08_ROM = (
	"08.double_height.nes",
	b"127fd966b6b32d6d88a53c5f59d7e938827783c9ad056091f119be1c4ab21c71"
)

TEST_09_ROM = (
	"09.timing_basics.nes",
	b"311698c717e50150edd0b5fd0016c41de686463205c20efb5630d6adb90859fd"
)

TEST_10_ROM = (
	"10.timing_order.nes",
	b"0f36bc07bfe51c416e3cc1a5231053572aa6b15aa60e6d2fd0568be49b6dc2e9"
)

TEST_11_ROM = (
	"11.edge_timing.nes",
	b"5a7c121f6e76617be88a0a7035c0e402293be5c685c95b97190a8d70835736ab"
)

DATA = [
	(TEST_01_ROM, TEST_01_BIN, '33'),
	(TEST_02_ROM, TEST_02_BIN, '31'),
	(TEST_03_ROM, TEST_03_BIN, '20'),
	(TEST_04_ROM, TEST_04_BIN, '18'),
	(TEST_05_ROM, TEST_05_BIN, '29'),
	(TEST_06_ROM, None, None),
	(TEST_07_ROM, None, None),
	(TEST_08_ROM, None, None),
	(TEST_09_ROM, None, None),
	(TEST_10_ROM, None, None),
	(TEST_11_ROM, None, None),
]