"""Regression guard for the Fortress of Winds mandibles crash."""
from pathlib import Path

source = Path(__file__).parents[1] / "src/projectile/mandiblesProjectile.c"
text = source.read_text(encoding="utf-8")
start = text.index("void MandiblesProjectile(")
end = text.index("void MandiblesProjectile_OnTick", start)
body = text[start:end]
assert "if (entity == NULL)" in body
assert "DeleteThisEntity();" in body and "return;" in body
assert "entity->confusedTime" in body

action3 = text.index("void MandiblesProjectile_Action3")
action4 = text.index("void MandiblesProjectile_Action4", action3)
assert "if (entity == NULL)" in text[action3:action4]
assert "return;" in text[action3:action4]
print("PASS: mandibles projectile handles deleted parent and child links")
