#include "pch.h"
#include "ScalingModesPage.h"
#include "EffectCatalog.h"
#include "EffectHelper.h"
#include "EffectsService.h"
#include "EffectChoiceItems.h"
#include <winrt/Windows.UI.Xaml.Automation.h>

using namespace ::Magpie;
using namespace winrt;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Input;
using Windows::UI::Xaml::Automation::AutomationProperties;

namespace winrt::Magpie::implementation {
namespace {
TextBlock PickerText(std::wstring_view text, double size = 14) {
	TextBlock block;
	block.Text(text);
	block.FontSize(size);
	block.TextWrapping(TextWrapping::Wrap);
	return block;
}
ScrollViewer PickerScroll(UIElement const& content) {
	ScrollViewer scroll;
	scroll.Content(content);
	scroll.HorizontalScrollMode(ScrollMode::Disabled);
	scroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
	scroll.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
	scroll.IsTabStop(false);
	return scroll;
}
Button PickerButton(UIElement const& content) {
	Button button;
	button.Content(content);
	button.HorizontalAlignment(HorizontalAlignment::Stretch);
	button.HorizontalContentAlignment(HorizontalAlignment::Stretch);
	button.Padding({ 10, 8, 10, 8 });
	button.BorderThickness({ 0 });
	button.Background(SolidColorBrush(Windows::UI::Color{ 0, 0, 0, 0 }));
	return button;
}
void PickerTip(DependencyObject const& target, std::wstring_view text) {
	auto block = PickerText(text, 13);
	block.MaxWidth(340);
	ToolTipService::SetToolTip(target, block);
}
}

void ScalingModesPage::_BuildEffectPicker() {
	_pickerRows.clear();
	const auto weak = get_weak();
	const auto& catalog = EffectCatalog::Get();
	_pickerRoot = Grid();
	_pickerRoot.ColumnSpacing(12);
	for (int i = 0; i < 2; ++i) _pickerRoot.ColumnDefinitions().Append(ColumnDefinition());
	for (int i = 0; i < 5; ++i) {
		RowDefinition row;
		row.Height({ 1, i == 3 ? GridUnitType::Star : GridUnitType::Auto });
		_pickerRoot.RowDefinitions().Append(row);
	}
	auto title = PickerText(L"添加效果器", 20);
	title.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
	title.Margin({ 0, 0, 0, 12 });
	Grid::SetColumnSpan(title, 2);
	_pickerRoot.Children().Append(title);
	_pickerSearch = TextBox();
	_pickerSearch.PlaceholderText(L"搜索名称、用途、中文关键词或旧名称……");
	AutomationProperties::SetName(_pickerSearch, L"搜索全部效果器");
	Grid::SetRow(_pickerSearch, 1);
	Grid::SetColumnSpan(_pickerSearch, 2);
	_pickerRoot.Children().Append(_pickerSearch);
	_pickerCount = PickerText(L"", 12);
	_pickerCount.Margin({ 0, 8, 0, 8 });
	Grid::SetRow(_pickerCount, 2);
	Grid::SetColumnSpan(_pickerCount, 2);
	_pickerRoot.Children().Append(_pickerCount);

	StackPanel categories;
	categories.Spacing(2);
	auto categoryButton = [&](StackPanel const& parent, std::wstring name, std::wstring category,
		std::wstring subcategory, std::wstring description) {
		auto button = PickerButton(PickerText(name, 13));
		PickerTip(button, description);
		AutomationProperties::SetName(button, name);
		button.Click([weak, category, subcategory, description](auto const&, auto const&) {
			if (auto page = weak.get()) page->_ChooseEffectCategory(category, subcategory, description);
		});
		auto show = [weak, name, description](auto const&, auto const&) {
			if (auto page = weak.get()) {
				page->_pickerDetailTitle.Text(name);
				page->_pickerDetails.Text(description);
				page->_pickerDetailScroll.ChangeView(nullptr, 0.0, nullptr, true);
			}
		};
		button.PointerEntered(show);
		button.GotFocus(show);
		parent.Children().Append(button);
	};
	categoryButton(categories, L"推荐入门", L"first_try", L"", L"按适用场景选择容易上手的效果器，先从一个开始比较画面。");
	categoryButton(categories, L"全部", L"", L"", L"查看所有已安装的效果器；搜索覆盖全部分类。");
	for (const auto& category : catalog.Categories()) {
		StackPanel children;
		children.Margin({ 12, 0, 0, 4 });
		children.Visibility(Visibility::Collapsed);
		StackPanel header;
		header.Orientation(Orientation::Horizontal);
		auto expand = PickerButton(PickerText(L"›", 16));
		expand.Width(28);
		expand.Padding({ 4, 4, 4, 4 });
		AutomationProperties::SetName(expand, L"展开或折叠" + category.name);
		expand.Click([children, expandWeak = make_weak(expand)](auto const&, auto const&) {
			const bool visible = children.Visibility() == Visibility::Visible;
			children.Visibility(visible ? Visibility::Collapsed : Visibility::Visible);
			if (auto button = expandWeak.get()) button.Content(PickerText(visible ? L"›" : L"⌄", 16));
		});
		header.Children().Append(expand);
		categoryButton(header, category.name, category.id, L"", category.description);
		header.Children().GetAt(1).as<Button>().Click([children, expandWeak = make_weak(expand)](auto const&, auto const&) {
			children.Visibility(Visibility::Visible);
			if (auto button = expandWeak.get()) button.Content(PickerText(L"⌄", 16));
		});
		categories.Children().Append(header);
		for (const auto& [name, description] : category.subcategories) {
			categoryButton(children, name, category.id, name, description);
		}
		categories.Children().Append(children);
	}
	categoryButton(categories, L"自定义／未归类", L"custom", L"", L"已安装但尚无用途说明的效果器。请参考作者说明选择参数和组合位置。");
	auto categoryScroll = PickerScroll(categories);
	Grid::SetRow(categoryScroll, 3);
	_pickerRoot.Children().Append(categoryScroll);

	Grid right;
	RowDefinition listRow, detailsRow;
	listRow.Height({ 3, GridUnitType::Star });
	detailsRow.Height({ 2, GridUnitType::Star });
	right.RowDefinitions().Append(listRow);
	right.RowDefinitions().Append(detailsRow);
	right.RowSpacing(10);
	Grid::SetColumn(right, 1);
	Grid::SetRow(right, 3);
	_pickerRoot.Children().Append(right);
	StackPanel results;
	results.Spacing(2);
	_pickerListScroll = PickerScroll(results);
	right.Children().Append(_pickerListScroll);
	StackPanel details;
	details.Spacing(8);
	_pickerDetailTitle = PickerText(L"用途与组合建议", 16);
	_pickerDetailTitle.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
	_pickerDetails = PickerText(L"悬停或键盘聚焦效果器，查看用途、使用条件和组合建议。", 13);
	_pickerDetails.IsTextSelectionEnabled(true);
	details.Children().Append(_pickerDetailTitle);
	details.Children().Append(_pickerDetails);
	_pickerDetailScroll = PickerScroll(details);
	Grid::SetRow(_pickerDetailScroll, 1);
	right.Children().Append(_pickerDetailScroll);

	bool includedRTX[2]{};
	for (const auto& effect : EffectsService::Get().Effects()) {
		const int family = RTXVideoFamily(effect.name);
		if (family >= 0 && includedRTX[family]) continue;
		const EffectInfo* selected = &effect;
		if (family >= 0) {
			includedRTX[family] = true;
			// Start with the lowest available tier; never change a saved stage's tier.
			for (auto id : RTX_VIDEO_IDS[family]) {
				if (auto installed = EffectsService::Get().GetEffect(id)) { selected = installed; break; }
			}
		}
		PickerRow row;
		if (const auto metadata = catalog.Find(selected->name)) row.entry = *metadata;
		else {
			row.entry.id = selected->name;
			row.entry.name = EffectHelper::GetDisplayName(selected->name);
			row.entry.category = L"custom";
			row.entry.summary = L"自定义效果器；请参考作者说明选择用途与组合。";
			row.entry.details = L"此效果器尚无用途 review。请参考作者提供的说明；添加后可调整参数与顺序。HDR 兼容性待验证。";
			row.entry.searchText = NormalizeEffectSearch(selected->name);
		}
		if (family >= 0) {
			for (auto id : RTX_VIDEO_IDS[family]) {
				if (const auto metadata = catalog.Find(id)) row.entry.searchText += metadata->searchText;
			}
		}
		_pickerRows.push_back(std::move(row));
	}
	std::ranges::sort(_pickerRows, {}, [](const PickerRow& row) { return NormalizeEffectSearch(row.entry.name); });
	for (size_t index = 0; index < _pickerRows.size(); ++index) {
		auto& row = _pickerRows[index];
		row.container = Grid();
		row.container.ColumnDefinitions().Append(ColumnDefinition());
		ColumnDefinition strengthColumn;
		strengthColumn.Width({ 1, GridUnitType::Auto });
		row.container.ColumnDefinitions().Append(strengthColumn);
		StackPanel content;
		content.Spacing(3);
		auto name = PickerText(row.entry.name);
		name.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
		content.Children().Append(name);
		std::wstring categoryName = L"自定义";
		for (const auto& category : catalog.Categories()) if (category.id == row.entry.category) categoryName = category.name;
		auto subtitle = PickerText(categoryName + L" · " + row.entry.recommendation, 11);
		subtitle.Opacity(0.75);
		content.Children().Append(subtitle);
		auto summary = PickerText(row.entry.summary, 12);
		summary.MaxLines(2);
		summary.TextTrimming(TextTrimming::CharacterEllipsis);
		content.Children().Append(summary);
		row.problem = PickerText(L"", 12);
		content.Children().Append(row.problem);
		row.button = PickerButton(content);
		AutomationProperties::SetName(row.button, row.entry.name);
		AutomationProperties::SetHelpText(row.button, row.entry.summary);
		PickerTip(row.button, row.entry.summary + L"\n详细说明见下方。单击追加到效果组末尾。");
		auto show = [weak, index](auto const&, auto const&) {
			if (auto page = weak.get()) page->_ShowEffectPickerDetails(index);
		};
		row.button.PointerEntered(show);
		row.button.GotFocus(show);
		row.button.Click([weak, index](auto const&, auto const&) {
			if (auto page = weak.get()) page->_AddPickedEffect(index);
		});
		row.button.KeyDown([weak, index](auto const&, KeyRoutedEventArgs const& args) {
			const auto page = weak.get();
			if (!page) return;
			if (args.Key() == VirtualKey::Enter) { args.Handled(true); page->_AddPickedEffect(index); }
			else if (args.Key() == VirtualKey::Down || args.Key() == VirtualKey::Up) {
				const int step = args.Key() == VirtualKey::Down ? 1 : -1;
				for (int i = static_cast<int>(index) + step; i >= 0 && i < static_cast<int>(page->_pickerRows.size()); i += step) {
					if (page->_pickerRows[i].container.Visibility() == Visibility::Visible) {
						page->_pickerRows[i].button.Focus(FocusState::Keyboard);
						args.Handled(true); break;
					}
				}
			}
		});
		row.container.Children().Append(row.button);
		const int family = RTXVideoFamily(row.entry.id);
		if (family >= 0) {
			ComboBox strength;
			strength.Width(82);
			strength.Margin({ 4, 8, 0, 0 });
			strength.VerticalAlignment(VerticalAlignment::Top);
			strength.Header(box_value(L"强度"));
			AutomationProperties::SetName(strength, row.entry.name + L" 强度");
			strength.ItemsSource(MakeEffectChoiceItems({ L"低", L"中", L"高", L"极高" }));
			strength.SelectedIndex(RTXVideoStrength(row.entry.id));
			strength.SelectionChanged([weak, index](auto const& sender, auto const&) {
				if (const auto page = weak.get()) {
					auto& entry = page->_pickerRows[index].entry;
					const auto id = RTXVideoId(entry.id, sender.template as<ComboBox>().SelectedIndex());
					if (!id.empty()) entry.id = id;
					page->_ShowEffectPickerDetails(index);
				}
			});
			strength.GotFocus(show);
			Grid::SetColumn(strength, 1);
			row.container.Children().Append(strength);
		}
		results.Children().Append(row.container);
	}
	auto footer = PickerText(L"单击效果器追加到末尾 · 悬停查看说明 · 添加后可拖动调整顺序", 12);
	footer.Margin({ 0, 12, 0, 0 });
	Grid::SetRow(footer, 4);
	Grid::SetColumnSpan(footer, 2);
	_pickerRoot.Children().Append(footer);
	_pickerSearch.TextChanged([weak](auto const&, auto const&) {
		if (const auto page = weak.get(); page && !page->_pickerChangingCategory) page->_RefreshEffectPicker();
	});
	_effectPicker = Flyout();
	_effectPicker.Content(_pickerRoot);
	Windows::UI::Xaml::Style presenter(xaml_typename<FlyoutPresenter>());
	presenter.Setters().Append(Setter(FrameworkElement::MaxWidthProperty(), box_value(1000.0)));
	presenter.Setters().Append(Setter(FrameworkElement::MaxHeightProperty(), box_value(900.0)));
	presenter.Setters().Append(Setter(Control::PaddingProperty(), box_value(Thickness{ 16 })));
	_effectPicker.FlyoutPresenterStyle(presenter);
	_effectPicker.Opened([weak](auto const&, auto const&) { if (auto page = weak.get()) page->_pickerSearch.Focus(FocusState::Programmatic); });
	_effectPicker.Closed([weak](auto const&, auto const&) { if (auto page = weak.get()) page->_pickerMode = nullptr; });
}

void ScalingModesPage::_ChooseEffectCategory(std::wstring category, std::wstring subcategory, std::wstring description) {
	_pickerCategory = std::move(category);
	_pickerSubcategory = std::move(subcategory);
	_pickerChangingCategory = true;
	_pickerSearch.Text(L"");
	_pickerChangingCategory = false;
	_RefreshEffectPicker();
	_pickerDetailTitle.Text(L"分类说明");
	_pickerDetails.Text(description);
}

void ScalingModesPage::_RefreshEffectPicker() {
	const auto query = _pickerSearch.Text();
	const bool searching = !NormalizeEffectSearch(query).empty();
	size_t count = 0;
	for (auto& row : _pickerRows) {
		const bool visible = searching ? MatchesEffectSearch(row.entry.searchText, query)
			: MatchesEffectCategory(row.entry, _pickerCategory, _pickerSubcategory);
		row.container.Visibility(visible ? Visibility::Visible : Visibility::Collapsed);
		if (visible) ++count;
		const auto problem = _pickerMode ? get_self<ScalingModeItem>(_pickerMode)->EffectAddProblem(hstring(row.entry.id)) : hstring{};
		row.problem.Text(problem);
		row.problem.Visibility(problem.empty() ? Visibility::Collapsed : Visibility::Visible);
		AutomationProperties::SetHelpText(row.button, problem.empty() ? hstring(row.entry.summary) : problem);
	}
	std::wstring scope = searching ? L"搜索全部分类" : L"全部";
	if (!searching && !_pickerCategory.empty()) {
		scope = _pickerCategory == L"first_try" ? L"推荐入门" : L"自定义／未归类";
		for (const auto& category : EffectCatalog::Get().Categories()) if (category.id == _pickerCategory) scope = category.name;
		if (!_pickerSubcategory.empty()) scope += L" › " + _pickerSubcategory;
	}
	_pickerCount.Text(scope + L" · " + std::to_wstring(count) + L" 个效果器");
	_pickerDetailTitle.Text(count ? L"用途与组合建议" : L"没有匹配的效果器");
	_pickerDetails.Text(count ? L"悬停或键盘聚焦效果器，查看用途、适用场景和组合建议。"
		: L"尝试缩短关键词、搜索算法家族名，或选择“全部”查看已安装效果器。");
	_pickerListScroll.ChangeView(nullptr, 0.0, nullptr, true);
}

void ScalingModesPage::_ShowEffectPickerDetails(size_t index) {
	if (index >= _pickerRows.size()) return;
	const auto& entry = _pickerRows[index].entry;
	const auto metadata = EffectCatalog::Get().Find(entry.id);
	_pickerDetailTitle.Text(entry.name);
	std::wstring text = metadata ? metadata->details : entry.details;
	if (_pickerMode) {
		const auto problem = get_self<ScalingModeItem>(_pickerMode)->EffectAddProblem(hstring(entry.id));
		if (!problem.empty()) text = std::wstring(problem) + L"\n\n" + text;
	}
	_pickerDetails.Text(text);
	_pickerDetailScroll.ChangeView(nullptr, 0.0, nullptr, true);
}

void ScalingModesPage::_AddPickedEffect(size_t index) {
	if (!_pickerMode || index >= _pickerRows.size()) return;
	const auto mode = _pickerMode;
	const hstring id(_pickerRows[index].entry.id);
	if (!get_self<ScalingModeItem>(mode)->EffectAddProblem(id).empty()) {
		_ShowEffectPickerDetails(index);
		return;
	}
	// Clear the target before dismissing so a second activation cannot append twice.
	_pickerMode = nullptr;
	_effectPicker.Hide();
	get_self<ScalingModeItem>(mode)->AddEffect(id);
}

}
