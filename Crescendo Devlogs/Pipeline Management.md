*// evil floating point bit level hacking*

Unlike OpenGL, Vulkan requires explicit creation of pipelines and their states to be used (Well not really anymore, we have dynamic render passes in 1.3). Pipelines have some of the largest configuration structures in Vulkan. Even tutorials such as [Vk-tutorial](https://vkguide.dev/) and [vkguide](https://vkguide.dev/) skirt around pipeline creation, because it's is a ton of work to create and manage so many pipelines.

Modern game engines go one step further, often creating hundreds of thousands, if not millions of pipelines waiting to be used, If you've ever gotten excited to play a large game that you've just downloaded, only to get stuck behind another loading screen that is compiling shaders, it's helpful to understand that the game is making thousands or millions of pipelines each with a different configuration state to make sure your experience is as smooth as possible. This is often a problem with some Godot games as they compile pipelines on the fly, creating random stutters.

While my engine will not be creating millions of pipelines as of yet, I still need a good system to manage different pipeline states. Lets jot down the requirements to figure out what system I want to build
## The requirements
Pipelines can have many different configurations, for now, I am interested in a small subset, each of the states I want to configure are as follows:
- Fill modes: Solid, Wireframe or Point
- Cull modes: Back, Front or None
- Multisamples: 1, 2, 4, 8, 16, 32 or 64
- Depth operators: Never, Less, Less-Equal, Equal, Greater, Greater-Equal or Always
- DepthTests: Enabled or Disabled
- DepthWrite: Enabled or Disabled

Given all these states, there are `3 * 3 * 5 * 7 * 2 * 2 = 1260` possible pipelines, Usually though we'd only care about a few, Most times we wouldn't need the point fill mode or a depth operator of always, so the unique number of pipelines that are practically used is much lower. It's a waste of resources and time to create all variants, so what if we had an intelligent system that only generated the pipelines we required
## The initial design
Lets consider a pipeline for a typical render pass
- Fill modes: Solid or Wireframe
- Cull Modes, Back or None
- Multisamples: 1, 2, 4
- Depth Operators: Less
- DepthTests: Enabled
- DepthWrite: Enabled or disabled

Hence there are `2 * 2 * 3 * 1 * 1 * 2 = 24` different pipelines needed this render pass. 
How can we go about creating each of these unique combinations? Well we could store each required state in a vector
```C++
std::vector<FillModes> fillModes = { FillModes::Solid, FillModes::WireFrame };
std::vector<CullModes> cullModes = { CullModes::Back, CullModes::None };
std::vector<Multisamples> multisamples = { MultiSamples::One, MultiSamples::Two, MultiSamples::Four };
std::vector<DepthFunc> depthFuncs = { DepthFunc::Less };
std::vector<DepthTest> depthTests = { DepthTest::Enabled };
std::vector<DepthWrite> depthWrites = { DepthWrite::Enabled, DepthWrite::Disabled };
```
To generate the pipelines, we could use nested loops, Cartesian product style
```C++
for (uint32_t i = 0; i < fillModes.size(); i++)
{
	for (uint32_t j = 0; j < cullModes.size(); j++)
	{
		for (uint32_t k = 0; k < multiSamples.size(); k++)
		{
			for (uint32_t l = 0; l < depthFuncs.size(); l++)
			{
				for (uint32_t m = 0; m < depthTests.size(); m++)
				{
					for (uint32_t n = 0; n < depthWrites.size(); n++)
					{
						// This isn't the method I use in the engine but you get the point
						pipelines.push_back(createPipeline(
							fillModes[i], cullModes[j], multiSamples[k], depthFuncs[l], depthTests[m], depthWrites[n]
						));
					}
				}	
			}
		}	
	}	
}
```
*Pyramid of doom*

Well that approach works but there are a few downsides to this:
### 1. It's ugly
Yes, you read that right. Having 6 nested for-loops is not only difficult to read but also cumbersome to manage. As more pipeline configurations are added, this structure will only become more convoluted and error-prone.
### 2. Order matters
In this system, the order of pipeline creation depends on the sequence of configuration states in the vectors. While pipelines could be stored in a map with a key representing each unique state, I wanted to hold each pipeline contiguously in a vector. This approach makes it challenging to map a subset of states to an index value, as it requires an additional system to track which pipeline corresponds to which state.
### 3. Not very scalable
As mentioned in point 1, this method is not scalable. For each new configuration, a new nested loop must be added. This not only increases technical debt but also complicates future extensions. We might even run out of variable names!

Given these issues, nested loops are clearly not the best solution. So, what alternatives do we have?
## A new approach: Bit flags
Bit flags provide a compact way to represent multiple boolean flags within a single variable. Each bit in the variable represents a different flag, allowing us to store and manipulate several flags efficiently. Typically, each bit has a corresponding constant that represents its position. This technique is widely used in Vulkan for various configurations and settings for their memory compactness and efficiency

For example we could store the `FillMode` in a bit flagged enum like so:
```C++
enum class FillMode : uint8_t { Solid = 0b1, Wireframe = 0b10, Point = 0b100 };
```
Hence if we wanted to represent the `FillMode` flags from the previous pipeline, it would be as simple as:
```C++
FillMode fillModes = FillMode::Solid | FillMode::WireFrame;
```
*Note that I have implemented bitwise operators for each of the enums*

So lets define enums for each of the pipelines states I want, Note that I do use uint8_t's because it's more than enough for each flag and explain later why
```C++
enum class FillMode : uint8_t { Solid = 0b1, Wireframe = 0b10, Point = 0b100 };
enum class CullMode : uint8_t { Back = 0b1, Front = 0b10, None = 0b100 };
enum class Multisamples : uint8_t { One = 0b1, Two = 0b10, Four = 0b100, Eight = 0b1000, Sixteen = 0b10000, ThirtyTwo = 0b100000, SixtyFour = 0b1000000 };
enum class DepthFunc : uint8_t { Never = 0b1, Less = 0b10, Equal = 0b100, LessEqual = 0b1000, Greater = 0b10000, GreaterEqual = 0b100000, Always = 0b1000000 };
enum class DepthTest : uint8_t { Enabled = 0b1, Disabled = 0b10 };
enum class DepthWrite : uint8_t { Enabled = 0b1, Disabled = 0b10 };
```
An additional benefit of using bit flags is that the order of pipeline states becomes irrelevant. Unlike the vector method, where the sequence matters, bit flags maintain a consistent bit order, ensuring each flag is always represented correctly regardless of the combination. This consistency simplifies management and reduces potential errors. (It is also a hint as to how it works without needing to track pipeline states)

And I have a simple macro that defines the bitwise operators for each one
```C++
#define CS_DEFINE_ENUM_CLASS_OR_OPERATOR(EnumName)\
	inline EnumName operator|(EnumName a, EnumName b) { return static_cast<EnumName>(static_cast<std::underlying_type_t<EnumName>>(a) | static_cast<std::underlying_type_t<EnumName>>(b)); };\
	inline EnumName operator|=(EnumName& a, EnumName b) { return a = a | b; }
#define CS_DEFINE_ENUM_CLASS_AND_OPERATOR(EnumName)\
	inline EnumName operator&(EnumName a, EnumName b) { return static_cast<EnumName>(static_cast<std::underlying_type_t<EnumName>>(a) & static_cast<std::underlying_type_t<EnumName>>(b)); };\
	inline EnumName operator&=(EnumName& a, EnumName b) { return a = a & b; }
#define CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EnumName) CS_DEFINE_ENUM_CLASS_OR_OPERATOR(EnumName); CS_DEFINE_ENUM_CLASS_AND_OPERATOR(EnumName)

// This defines the bitwise OR | and bitwise AND & operators for this enum
CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(FillMode);
```

Since we have 6 different types of flags, we can store them together in one class or struct, I use a union with a uint64_t to pad it out to 8 bytes since it aligns much nicer in memory and leaves us with extra flags we can use later for completely free. It also means that a pipeline configuration can be uniquely represented with a 64-bit integer, this can be potentially useful for hash keys.
```C++
class PipelineVariants
{
private:
	union {
		// usually 6 bytes with 1 byte alignment
		struct {
			FillMode fillModeFlags;
			CullMode cullModeFlags;
			Multisamples multisampleFlags;
			DepthFunc depthFuncFlags;
			DepthTest depthTestFlags;
			DepthWrite depthWriteFlags;
		};
		// With this union, it becomes 8 bytes with 8 byte alignment
		uint64_t flags;
	};
}
```
Still, enum names can get long and having to bitwise them can be annoying, so I implement a few static method to generate commonly used pipeline configurations. In my engine I have several passes, a depth-prepass, an offscreen pass and a post-processing pass, each one requiring different pipeline configurations
```C++
PipelineVariants PipelineVariants::GetDefaultVariant(uint32_t samples)
{
	return PipelineVariants(FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Disabled);
}
PipelineVariants PipelineVariants::GetDepthPrepassVariant(uint32_t samples)
{
	return PipelineVariants(FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Enabled);
}
PipelineVariants PipelineVariants::GetPostProcessingVariant(uint32_t samples)
{
	return PipelineVariants(FillMode::Solid, CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::Always, DepthTest::Disabled, DepthWrite::Disabled);
}
```
*A few of the variants I use in my engine*
## Implementation
Well that's good and all but how do we extract different pipelines from this?

First we need a method that counts how many variants a pipeline will have, from here on out we will use the pipeline flags in the original example. With the vector method, the number of pipelines is equal to the product of the length of each vector. Doing it with bit flags however, it is then equal to the product of the number of 1 bits in the flag, this is easily achieved with `std::popcount` in the `<bit>` header
```C++
uint32_t PipelineVariants::GetVariantCount() const
{
	return
		std::popcount(static_cast<uint8_t>(fillModeFlags)) *
		std::popcount(static_cast<uint8_t>(cullModeFlags)) *
		std::popcount(static_cast<uint8_t>(multisampleFlags)) *
		std::popcount(static_cast<uint8_t>(depthFuncFlags)) *
		std::popcount(static_cast<uint8_t>(depthTestFlags)) *
		std::popcount(static_cast<uint8_t>(depthWriteFlags));
}
```
*Interesting bit of trivia, popcount was originally a compiler specific macro that was then later added to the official C++ spec, hence it is now safe to use cross-platform

Secondly, we need a method to verify if a pipeline configuration is valid for a given `PipelineVariant`. This can be efficiently accomplished by checking if each state has a corresponding valid flag using bitwise `&` operators. Simply perform a bitwise `&` operation between the configuration and the variant flag; if all required flags match, then we can easily determine that the variant exists.
```C++
bool PipelineVariants::VariantExists(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const
{
	return
		static_cast<uint8_t>(fillModeFlags & fillMode) &&
		static_cast<uint8_t>(cullModeFlags & cullMode) &&
		static_cast<uint8_t>(multisampleFlags & multisamples) &&
		static_cast<uint8_t>(depthFuncFlags & depthFunc) &&
		static_cast<uint8_t>(depthTestFlags & depthTest) &&
		static_cast<uint8_t>(depthWriteFlags & depthWrite);
}
```

Thirdly we need one more method. This method returns the nth 1 bit, where the 1st bit is the least significant bit. It is used to isolate specific flags in a given bit flag. Using this method, along with `std::popcount` lets us easily extract a specific flag in a variant. If the nth 1 could not be found, we gracefully fail with a 0
```C++
template <typename T>
// Finds the value of the nth bit set to 1 in a number
constexpr T getNthSetBit(T number, T n)
{
	T count = 0;
	for (T i = 0; i < 8 * sizeof(T); i++)
	{
		if (number & (1 << i))
		{
			if (count == n) return (1 << i);
			count++;
		}
	}
	return 0;
}
```
It's a bit hard to explain so let me provide an example
```
Say we have an 8-bit number 0b10011011,
If we are looking for the 3rd 1 bit (LSB), it would return the value:
0b00001000, the 3rd 1 in the number
```
## That's cool and all but aren't you beating around the bush?
We'll get to it, these functions are important to how this method works. there's two ways I want to be able to get a variant. I will explain one in this article because it can easily be derivable and I am too lazy to write more.
- Either get it's index via a set of flags
- Or get a variant from an index (I'll leave this as an exercise to the reader)
## Getting a variant's index via a set of flags
```C++
uint32_t PipelineVariants::GetVariantIndex(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const
{
	// If the variant doesn't exist, return some invalid index, I doubt this will ever be used
	if (!VariantExists(fillMode, cullMode, multisamples, depthFunc, depthTest, depthWrite)) return std::numeric_limits<uint32_t>::max();

	uint32_t index = 0;
	uint32_t multiplier = 1;

	const auto updateIndexWithFlag = [&](auto flag, auto allFlags)
	{
		uint8_t totalFlags = std::popcount(static_cast<uint8_t>(allFlags));
		uint8_t currentFlagPosition = std::popcount(static_cast<uint8_t>((static_cast<uint8_t>(flag) - 1) & static_cast<uint8_t>(allFlags)));
		index += multiplier * currentFlagPosition;
		multiplier *= totalFlags;
	};

	// MUST BE DONE IN REVERSE ORDER
	updateIndexWithFlag(depthWrite, depthWriteFlags);
	updateIndexWithFlag(depthTest, depthTestFlags);
	updateIndexWithFlag(depthFunc, depthFuncFlags);
	updateIndexWithFlag(multisamples, multisampleFlags);
	updateIndexWithFlag(cullMode, cullModeFlags);
	updateIndexWithFlag(fillMode, fillModeFlags);

	return index;
}
```
*This is best shown in full at first, then explained in steps*

We use the `VariantExists` method to first validate the set of flags. If the configuration doesn't pass the check, the default behavior gracefully handles this by returning the maximum integer value, signaling an invalid configuration. This approach ensures that only valid pipeline states are processed.
```C++
// If the variant doesn't exist, return some invalid index, I doubt this will ever be used
if (!VariantExists(fillMode, cullMode, multisamples, depthFunc, depthTest, depthWrite)) return std::numeric_limits<uint32_t>::max();
```
Next up we define a few variables and a lambda that we will use
```C++
uint32_t index = 0;
uint32_t multiplier = 1;

const auto addFlagToIndex = [&](auto flag, auto allFlags)
{
	uint8_t totalFlags = std::popcount(static_cast<uint8_t>(allFlags));
	uint8_t currentFlagPosition = std::popcount(static_cast<uint8_t>((static_cast<uint8_t>(flag) - 1) & static_cast<uint8_t>(allFlags)));
	index += multiplier * currentFlagPosition;
	multiplier *= totalFlags;
};
```
`addFlagToIndex` is an interesting method, lets break it down
### Function signature
```C++
const auto addFlagToIndex = [&](auto flag, auto allFlags)
```
- The function signature has two arguments, `flag` and `allFlags`
	- `flag` is the specific flag we want to isolate, in this example `PolygonMode::Solid`
	- `allFlags` denotes the specific bit flag structure storing all the flags, in this example being `PolygonMode::Solid | PolygonMode::Wireframe`
The function then counts the total number of flags using `popcount` again and stores it in a variable `totalFlags`. this becomes the multiplier and denotes the index offset between each flag.

Next, `currentFlagPosition` determines the position of the current `flag` within `allFlags`. It does this by counting the number of `1` bits in the bitwise AND of `(flag - 1)` and `allFlags`. The subtraction by 1 shifts the focus to the preceding flags, ensuring we count all bits before the current `flag`. For example if `flag` is `FillMode::Wireframe` (binary `0b10`) and `allFlags` is `FillMode::Solid | FillMode::Wireframe` (binary `0b11`), the calculation would determine that `Wireframe` is the second active flag (since `Solid` is the first). Then we increment `index` by `multiplier * currentFlagPosition`, `multiplier` is the offset relative to each other combination before it, while `currentFlagPosition`  denotes the specific flag it references.

Finally, we multiply `multiplier` by `totalFlags` to get the next offset for the next set of pipeline states. That way that indices map to a specific variant is through combinations, lets try a small subset of flags, just the `DepthTest` and `DepthFunc` to explain this.

`DepthTest` can take on 2 values: `Enabled` or `Disabled`
`DepthFunc` can take on 7 values: `Never`, `Less`, `Equal`, `LessEqual`, `Greater`, `GreaterEqual`, `Always`

Lets assume a pipeline variation like so:
```C++
PipelineVariants(
	FillMode::Solid,
	CullMode::Back,
	Multisamples:One,
	DepthFunc::Less | DepthFunc::Equal | DepthFunc::GreaterEqual,
	DepthTest::Enabled,
	DepthWrite::Disabled | DepthWrite::Enabled);
```

This pipeline has `1 * 1 * 1 * 3 * 1 * 2 = 6` configurations, hence 6 indices

Since `DepthTest` comes last in the flags, it corresponds to the least significant "bits", hence:
- Index 0 corresponds to a pipeline with `DepthWrite::Enabled`
- Index 1 corresponds to a pipeline with `DepthWrite::Disabled`
- Index 2 is `DepthWrite::Enabled`
- Index 3 is` DepthWrite::Disabled`
- Index 4 is `DepthWrite::Enabled`
- Index 5 is `DepthWrite::Disabled`
You can see how it is supposed to alternate between the two, 

Lets try with `DepthFunc`
- Index 0 is `DepthFunc::Less`
- Index 1 is `DepthFunc::Less`
- Index 2 is `DepthFunc::Equal`
- Index 3 is `DepthFunc::Equal`
- Index 4 is `DepthFunc::GreaterEqual`
- Index 5 is `DepthFunc::GreaterEqual`
`DepthFunc` changes every two pipelines, notice how for each `DepthFunc`, there is also both `DepthWrites`. This is the key component in making this work. 

```C++
// MUST BE DONE IN REVERSE ORDER
addFlagToIndex(depthWrite, depthWriteFlags);
addFlagToIndex(depthTest, depthTestFlags);
addFlagToIndex(depthFunc, depthFuncFlags);
addFlagToIndex(multisamples, multisampleFlags);
addFlagToIndex(cullMode, cullModeFlags);
addFlagToIndex(fillMode, fillModeFlags);
```
There is a key component here, noted by the comment `// MUST BE DONE IN REVERSE ORDER`. This is because the last arguments in the pipeline variants denote the least significant "bits", with each one leading to the first leading to a more successive "bit". Hence after all this, we obtain our unique index for this pipeline.