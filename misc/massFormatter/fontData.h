constexpr int8_t dw_fnt_8bit_sprite_font_character_widths[] = {5,1,3,11,5,7,7,1,5,5,9,5,2,3,2,4,9,5,9,9,9,9,9,9,9,9,1,2,5,5,5,5,6,9,9,9,9,9,9,9,9,4,9,9,9,14,9,9,9,9,9,9,9,9,9,14,9,9,9,5,4,5,5,7,2,9,9,9,9,9,9,9,9,4,9,9,9,14,9,9,9,9,9,9,9,9,9,14,9,9,9,4,2,4,5,1};

constexpr bn::utf8_character dw_fnt_8bit_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_8bit_sprite_font_utf8_characters_span(dw_fnt_8bit_sprite_font_utf8_characters);

constexpr auto dw_fnt_8bit_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_8bit_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_8bit_sprite_font(
        bn::sprite_items::dw_fnt_8bit,
		dw_fnt_8bit_sprite_font_utf8_characters_map.reference(),
        dw_fnt_8bit_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_digital_sprite_font_character_widths[] = {7,3,7,9,7,9,7,3,4,4,7,8,3,7,3,9,7,5,7,7,7,7,7,7,7,7,3,3,5,7,5,7,9,7,7,7,7,7,7,7,7,5,7,7,7,9,8,7,7,8,7,7,8,7,7,9,7,8,7,5,9,5,8,8,5,7,7,7,7,7,5,7,7,3,4,7,4,11,7,7,7,7,7,7,5,7,7,11,7,7,7,5,3,5,9,1};

constexpr bn::utf8_character dw_fnt_digital_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_digital_sprite_font_utf8_characters_span(dw_fnt_digital_sprite_font_utf8_characters);

constexpr auto dw_fnt_digital_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_digital_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_digital_sprite_font(
        bn::sprite_items::dw_fnt_digital,
		dw_fnt_digital_sprite_font_utf8_characters_map.reference(),
        dw_fnt_digital_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_eb_sprite_font_character_widths[] = {4,2,5,7,7,9,10,2,3,3,6,5,2,5,1,7,6,5,5,6,6,5,6,6,5,6,1,2,7,7,7,5,11,9,7,7,7,7,6,8,8,3,5,8,7,12,8,8,6,9,8,6,9,8,10,13,10,9,8,3,7,3,6,7,3,6,7,6,7,5,6,7,8,3,4,7,3,11,8,6,6,7,5,4,5,8,8,11,8,8,5,4,1,4,7,1};

constexpr bn::utf8_character dw_fnt_eb_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_eb_sprite_font_utf8_characters_span(dw_fnt_eb_sprite_font_utf8_characters);

constexpr auto dw_fnt_eb_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_eb_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_eb_sprite_font(
        bn::sprite_items::dw_fnt_eb,
		dw_fnt_eb_sprite_font_utf8_characters_map.reference(),
        dw_fnt_eb_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_etext_12_sprite_font_character_widths[] = {3,1,3,6,6,6,7,1,4,5,3,4,1,4,1,6,6,2,6,6,6,6,6,6,6,6,1,2,6,4,7,6,8,6,6,5,6,6,6,6,6,6,6,6,6,8,6,6,6,7,6,6,6,6,6,11,6,6,6,5,6,5,6,6,2,6,6,5,6,6,5,6,6,1,4,6,2,6,6,6,6,6,5,6,4,6,6,6,6,6,6,5,1,5,5,1};

constexpr bn::utf8_character dw_fnt_etext_12_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_etext_12_sprite_font_utf8_characters_span(dw_fnt_etext_12_sprite_font_utf8_characters);

constexpr auto dw_fnt_etext_12_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_etext_12_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_etext_12_sprite_font(
        bn::sprite_items::dw_fnt_etext_12,
		dw_fnt_etext_12_sprite_font_utf8_characters_map.reference(),
        dw_fnt_etext_12_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_extra_12_sprite_font_character_widths[] = {6,2,6,8,7,7,8,2,3,3,6,7,2,6,2,7,7,3,7,7,7,7,7,7,7,7,2,2,7,7,7,7,8,7,7,7,7,7,7,7,7,2,7,8,6,8,8,7,7,8,7,7,7,7,7,8,8,7,7,3,7,3,6,8,3,7,7,7,7,7,7,7,7,2,5,7,2,8,7,7,7,8,7,7,6,7,7,8,8,7,8,5,2,5,8,1};

constexpr bn::utf8_character dw_fnt_extra_12_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_extra_12_sprite_font_utf8_characters_span(dw_fnt_extra_12_sprite_font_utf8_characters);

constexpr auto dw_fnt_extra_12_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_extra_12_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_extra_12_sprite_font(
        bn::sprite_items::dw_fnt_extra_12,
		dw_fnt_extra_12_sprite_font_utf8_characters_map.reference(),
        dw_fnt_extra_12_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_fixed_sprite_font_character_widths[] = {7,5,8,13,13,13,13,3,13,13,13,13,3,13,3,13,13,13,13,13,13,13,13,13,13,13,3,3,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,11,13,11,7,13,5,13,13,13,13,13,13,13,13,13,12,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,4,13,13,1};

constexpr bn::utf8_character dw_fnt_fixed_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_fixed_sprite_font_utf8_characters_span(dw_fnt_fixed_sprite_font_utf8_characters);

constexpr auto dw_fnt_fixed_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_fixed_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_fixed_sprite_font(
        bn::sprite_items::dw_fnt_fixed,
		dw_fnt_fixed_sprite_font_utf8_characters_map.reference(),
        dw_fnt_fixed_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_org_sprite_font_character_widths[] = {12,2,6,10,10,10,10,2,4,4,6,6,2,8,2,10,10,2,10,10,10,10,10,10,10,10,2,2,6,8,6,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,4,10,4,6,10,2,8,8,8,8,8,6,8,8,2,4,8,2,10,8,8,8,8,8,8,10,8,8,10,8,8,8,6,2,6,10,1};

constexpr bn::utf8_character dw_fnt_org_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_org_sprite_font_utf8_characters_span(dw_fnt_org_sprite_font_utf8_characters);

constexpr auto dw_fnt_org_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_org_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_org_sprite_font(
        bn::sprite_items::dw_fnt_org,
		dw_fnt_org_sprite_font_utf8_characters_map.reference(),
        dw_fnt_org_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_past2_sprite_font_character_widths[] = {3,2,3,5,5,6,6,1,3,3,5,5,2,4,2,3,5,3,5,5,5,5,5,5,5,5,2,2,4,4,4,5,6,5,5,5,5,5,5,5,5,2,4,6,4,7,6,6,5,6,5,4,6,5,6,7,6,6,5,3,3,3,3,6,2,5,5,4,5,5,3,5,5,2,3,5,2,9,5,5,5,5,4,4,3,5,5,7,5,5,5,4,1,4,5,1};

constexpr bn::utf8_character dw_fnt_past2_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_past2_sprite_font_utf8_characters_span(dw_fnt_past2_sprite_font_utf8_characters);

constexpr auto dw_fnt_past2_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_past2_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_past2_sprite_font(
        bn::sprite_items::dw_fnt_past2,
		dw_fnt_past2_sprite_font_utf8_characters_map.reference(),
        dw_fnt_past2_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_suomi_12_sprite_font_character_widths[] = {3,1,3,5,5,8,7,1,3,3,3,5,1,3,1,3,5,4,5,5,5,5,5,5,5,5,1,1,5,5,4,3,8,8,6,6,8,6,6,7,8,3,4,8,6,10,8,7,6,10,7,5,7,8,8,11,8,7,7,2,3,2,5,8,2,5,5,4,6,4,4,5,6,3,2,6,3,9,6,5,5,6,4,4,3,6,5,8,5,5,5,3,1,3,5,1};

constexpr bn::utf8_character dw_fnt_suomi_12_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_suomi_12_sprite_font_utf8_characters_span(dw_fnt_suomi_12_sprite_font_utf8_characters);

constexpr auto dw_fnt_suomi_12_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_suomi_12_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_suomi_12_sprite_font(
        bn::sprite_items::dw_fnt_suomi_12,
		dw_fnt_suomi_12_sprite_font_utf8_characters_map.reference(),
        dw_fnt_suomi_12_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_text_12_sprite_font_character_widths[] = {5,2,5,7,6,8,8,2,4,4,7,6,2,5,2,4,7,4,7,6,7,6,6,7,6,6,2,2,7,6,7,6,8,7,7,7,7,7,6,7,7,3,6,8,7,11,7,8,7,8,8,7,8,8,7,9,7,7,7,3,4,3,5,7,3,7,7,5,7,5,4,6,6,3,3,7,4,8,6,6,6,6,5,5,4,7,5,8,5,6,6,6,2,6,8,1};

constexpr bn::utf8_character dw_fnt_text_12_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_text_12_sprite_font_utf8_characters_span(dw_fnt_text_12_sprite_font_utf8_characters);

constexpr auto dw_fnt_text_12_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_text_12_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_text_12_sprite_font(
        bn::sprite_items::dw_fnt_text_12,
		dw_fnt_text_12_sprite_font_utf8_characters_map.reference(),
        dw_fnt_text_12_sprite_font_character_widths, 1);

		constexpr int8_t dw_fnt_times_sprite_font_character_widths[] = {4,2,4,8,6,12,11,1,4,5,6,9,2,4,2,4,6,4,7,6,7,6,6,6,6,6,2,2,9,9,9,5,13,11,10,9,11,9,8,10,11,5,6,12,9,14,11,10,8,10,11,7,9,11,11,15,11,11,9,4,4,3,7,8,3,6,7,6,7,6,6,8,8,4,4,8,4,12,8,6,7,7,5,5,4,8,8,11,8,8,7,5,1,5,9,1};

constexpr bn::utf8_character dw_fnt_times_sprite_font_utf8_characters[] = {"Á"}; // dont ask

constexpr bn::span<const bn::utf8_character> dw_fnt_times_sprite_font_utf8_characters_span(dw_fnt_times_sprite_font_utf8_characters);

constexpr auto dw_fnt_times_sprite_font_utf8_characters_map =
        bn::utf8_characters_map<dw_fnt_times_sprite_font_utf8_characters_span>();

constexpr bn::sprite_font dw_fnt_times_sprite_font(
        bn::sprite_items::dw_fnt_times,
		dw_fnt_times_sprite_font_utf8_characters_map.reference(),
        dw_fnt_times_sprite_font_character_widths, 1);

		