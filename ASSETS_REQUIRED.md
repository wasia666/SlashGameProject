# 外部素材依赖清单 (Assets Required)

本仓库**不包含**第三方素材库的原始文件（体积过大，且受各家许可协议约束）。
下面列出的是：要让 `Content/Map/NewMap.umap` 与全部蓝图完整还原，**必须重新导入**的素材。

> 数据来源：对项目自有资源（蓝图 / 地图 / 关卡外部 Actor / HUD / 特效）做 `/Game/` 引用
> 递归闭包分析得到的精确结果。

## 总览

- 项目**实际用到**的外部素材：**1892 个资源包，30.12 GB**
- 这些素材库的**完整目录**合计：31617 个文件，157.49 GB
- 其中**完全未被引用**、可直接忽略的：Stump_Scanned、SwordTrailVFX、AnimeCharacters、Mobile_Trees

## 需要重新导入的素材库

| 素材库 | 直接引用 | 需导入资源(含依赖) | 需导入体积 | 库总体积 | 使用率 |
|---|---:|---:|---:|---:|---:|
| `AncientContent` | 319 | 912 | 24.29 GB | 31.73 GB | 76.5% |
| `MedievalDungeon` | 133 | 368 | 1.89 GB | 2.09 GB | 90.4% |
| `OldWest` | 22 | 98 | 1.07 GB | 1.63 GB | 65.8% |
| `PN_interactiveSpruceForest` | 20 | 132 | 997.2 MB | 1.46 GB | 66.8% |
| `Texture` | 30 | 30 | 502.4 MB | 1.51 GB | 32.5% |
| `ForestLandscape` | 12 | 12 | 340.7 MB | 1.54 GB | 21.6% |
| `PN_coniferBushes_I` | 26 | 68 | 303.6 MB | 471.0 MB | 64.5% |
| `PN_Pandanus` | 21 | 61 | 225.3 MB | 257.3 MB | 87.5% |
| `AncientTreasures` | 10 | 61 | 211.5 MB | 919.4 MB | 23.0% |
| `Mixamo` | 24 | 29 | 181.8 MB | 209.6 MB | 86.7% |
| `PN_mediterraneanPlants_I` | 12 | 40 | 86.5 MB | 629.2 MB | 13.7% |
| `StarterContent` | 11 | 28 | 49.1 MB | 628.7 MB | 7.8% |
| `AnimalVarietyPack` | 1 | 9 | 21.5 MB | 493.9 MB | 4.3% |
| `MedievalSword` | 1 | 8 | 10.4 MB | 18.0 MB | 57.7% |
| `ParagonMinions` | 0 | 4 | 2.0 MB | 4.76 GB | 0.0% |
| `VaultCache` | 8 | 8 | 1.5 MB | 107.05 GB | 0.0% |
| `Weapon_Pack` | 3 | 3 | 0.3 MB | 1.15 GB | 0.0% |
| `RockEnv_Pack` | 22 | 21 | 0.1 MB | 131.6 MB | 0.1% |

> 「直接引用」为 0 表示该库没有被项目自有资源直接引用，只是被其它外部资源**间接**带进来的
> （例如 `VaultCache`、`ParagonMinions`）。这类资源实际占用很小，可酌情忽略。

## 完全未使用（无需下载）

| 素材库 | 文件数 | 体积 |
|---|---:|---:|
| `Stump_Scanned` | 63 | 648.7 MB |
| `SwordTrailVFX` | 100 | 164.5 MB |
| `AnimeCharacters` | 196 | 100.1 MB |
| `Mobile_Trees` | 9 | 20.6 MB |

## 精确清单

`ASSETS_MANIFEST.txt` 列出了每一个需要还原的资源包路径（按素材库分组），
可用于比对导入结果是否完整。

## 导入步骤

1. 用 **UE 5.0** 打开本工程（直接双击 `slash2.uproject`）。
2. 在 Fab / Epic Games Launcher 的 **Library** 中获取上表列出的素材包，
   分别 **Add to Project → 选择本工程**，保持导入到 `Content/<同名目录>`。
3. 若导入路径与清单不一致，地图与蓝图会出现资源丢失（Missing）提示——
   对照 `ASSETS_MANIFEST.txt` 逐个修正即可。
4. `Content/Mixamo/` 中的角色与动画来自 **Mixamo**，其 FBX 源文件已随仓库提供，
   见 `Assests/Mixamo/`，可直接重新导入。

## 说明

- `Content/Texture/` 为 Megascans 4K 地表贴图；`Content/AncientContent/` 为 MASS 巨石/悬崖高模
  （单文件最大 549 MB），是体积占用的主要来源。
- 若只需要阅读/运行游戏逻辑，`Source/` + `Content/Blueprints/` + `Config/` 已完全自洽；
  缺少的仅是地图的**视觉呈现**。
