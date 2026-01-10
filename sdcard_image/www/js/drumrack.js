// drumrack.js - Dynamic UI and API logic for DrumRack
$(async function() {
    const channel = 0; // Default channel, can be made selectable
    const PARAMS_URL = `/api/v1/getPluginParams/${channel}`;
    const SET_PARAM_URL = `/api/v1/setPluginParam/${channel}`;

    // Utility: fetch JSON
    async function fetchJSON(url) {
        const res = await fetch(url);
        return await res.json();
    }

    // Utility: debounce
    function debounce(fn, delay) {
        let t; return function(...args) {
            clearTimeout(t); t = setTimeout(() => fn.apply(this, args), delay);
        };
    }

    // Check if DrumRack is active
    const activePlugin = await fetchJSON(`/api/v1/getActivePlugin/${channel}`);
    if (!activePlugin || activePlugin.id !== 'DrumRack') {
        // Hide preset icon bar and show message only
        $('#preset-icon-bar').hide();
        $('#drumrack-ui').html('<div style="font-size:1.5em;color:#b71c1c;text-align:center;margin-top:3em;">DrumRack not active!</div>');
        return;
    } else {
        $('#preset-icon-bar').show();
    }

    // Load parameter values from API
    const paramVals = await fetchJSON(PARAMS_URL);

    // Map current values by id
    const currentMap = {};
    function mapCurrent(params) {
        params.forEach(p => {
            if (p.type === 'group') mapCurrent(p.params);
            else currentMap[p.id] = p.current;
        });
    }
    mapCurrent(paramVals.params);

    // Helper: find param value by id in paramVals
    function findParamVal(params, id) {
        for (const p of params) {
            if (p.type === 'group') {
                const found = findParamVal(p.params, id);
                if (found) return found;
            } else if (p.id === id) {
                return p;
            }
        }
        return null;
    }

    // Build UI
    function createParamControl(param) {
        const val = currentMap[param.id] ?? param.min ?? 0;
        let html = '';
        const tooltip = param.hint ? `<span class="param-tooltip" title="${param.hint}">?</span>` : '';
        // Mapping indicator: show red dot if mapped to CV or TRIG
        let mappingActive = false;
        let cv = (typeof param.cv !== 'undefined') ? param.cv : undefined;
        let trig = (typeof param.trig !== 'undefined') ? param.trig : undefined;
        if ((typeof cv !== 'undefined' && cv !== -1) || (typeof trig !== 'undefined' && trig !== -1)) {
            mappingActive = true;
        }
        const mappingDot = `<span class="mapping-indicator" id="mapping-indicator-${param.id}" style="display:${mappingActive ? 'inline-block' : 'none'}"></span>`;
        if (param.type === 'bool' && param.id.endsWith('_trigger')) {
            html = `<div class="drumrack-param"><button type="button" class="trigger-btn" data-id="${param.id}" title="${param.name}${param.hint ? ' - ' + param.hint : ''}">Trigger</button>${mappingDot}</div>`;
        } else if (param.type === 'bool') {
            html = `<div class="drumrack-param"><label><input type="checkbox" data-id="${param.id}" ${val ? 'checked' : ''}/> ${param.name}${mappingDot}${tooltip}</label></div>`;
        } else if (param.type === 'int' || param.type === 'float') {
            const min = param.min ?? 0, max = param.max ?? 4095, step = param.step ?? 1;
            html = `<div class="drumrack-param"><label for="${param.id}">${param.name}${mappingDot}${tooltip}<span class="param-value param-editable" id="val-${param.id}" data-id="${param.id}" title="Double click to edit">${val}</span></label><input type="range" data-id="${param.id}" id="${param.id}" min="${min}" max="${max}" step="${step}" value="${val}"/></div>`;
        }
        return html;
    }

    function createGroup(group) {
        let html = `<section class="drumrack-group" id="group-${group.id}">
            <div class="drumrack-group-header"><span class="toggle">&#9654;</span> ${group.name}`;
        // Add help icon for all top-level groups (not for nested groups)
        if (!group.id.startsWith('group-')) {
            html += ` <img src="img/help_outline.svg" class="drumrack-group-help" data-group-id="${group.id}" title="Help" style="width:1.1em;height:1.1em;vertical-align:middle;cursor:pointer;margin-left:0.5em;filter:brightness(0) saturate(100%);"/>`;
        }
        html += `</div><div class="drumrack-params">`;
        group.params.forEach(param => {
            if (param.type === 'group') html += createGroup(param);
            else html += createParamControl(param);
        });
        html += '</div></section>';
        return html;
    }

    // Dynamically build group order from paramVals
    function buildGroupOrder(params) {
        // Try to group by id suffixes (e.g. *_group)
        const groupSections = [];
        const groupMap = {};
        params.forEach(g => {
            if (g.type === 'group') {
                let label = g.name;
                // Try to classify by id
                if (g.id.match(/b_group/)) label = 'Bass Drums';
                else if (g.id.match(/s_group/)) label = 'Snares';
                else if (g.id.match(/hh/)) label = 'HiHats';
                else if (g.id.match(/cl|rs/)) label = 'Percussion';
                else if (g.id.match(/s[1-4]_group/)) label = 'Romplers';
                else if (g.id.match(/fx/)) label = 'FX';
                else if (g.id.match(/c_group/)) label = 'Compressor';
                else if (g.id === 'sum') label = 'Overall Mix';
                if (!groupMap[label]) groupMap[label] = [];
                groupMap[label].push(g);
            }
        });
        for (const label in groupMap) {
            groupSections.push({label, groups: groupMap[label]});
        }
        return groupSections;
    }

    // --- UI: Add Load/Save Preset Buttons ---
    let ui = '';
    ui += '<div class="drumrack-groups-grid">';
    const groupSections = buildGroupOrder(paramVals.params);
    groupSections.forEach(section => {
        ui += `<div class="drumrack-section"><div class="drumrack-section-label">${section.label}</div>`;
        section.groups.forEach(group => {
            ui += createGroup(group);
        });
        ui += '</div>';
    });
    ui += '</div>';
    $('#drumrack-ui').append(ui);

    // --- Modal for Load/Save Preset ---
    if (!$('#drumrack-preset-modal').length) {
      $('body').append(`
        <div id="drumrack-preset-modal" style="display:none;position:fixed;z-index:1000;left:0;top:0;width:100vw;height:100vh;background:rgba(0,0,0,0.3);align-items:center;justify-content:center;">
          <div id="drumrack-preset-modal-content" style="background:#fff;padding:2em 2em 1em 2em;border-radius:10px;min-width:300px;max-width:90vw;"></div>
        </div>
      `);
    }

    // --- Load Preset Handler ---
    $('#drumrack-load-preset').on('click', function() {
      $('#drumrack-preset-modal-content').html('<div>Loading presets...</div>');
      $('#drumrack-preset-modal').fadeIn(100);
      $.get('/api/v1/getPresets/' + channel, function(data) {
        if(typeof data === 'string') data = JSON.parse(data);
        let html = `<h3>Load Preset</h3><ul style='list-style:none;padding:0;'>`;
        data.presets.forEach(p => {
          html += `<li><button class='drumrack-preset-load-btn' data-num='${p.number}'>${p.number}: ${p.name}${data.activePresetNumber==p.number ? ' <b>(active)</b>' : ''}</button></li>`;
        });
        html += '</ul><button id="drumrack-preset-modal-cancel">Cancel</button>';
        $('#drumrack-preset-modal-content').html(html);
        $('.drumrack-preset-load-btn').on('click', function() {
          const num = $(this).data('num');
          $('#drumrack-preset-modal-content').html('Loading...');
          $.get('/api/v1/loadPreset/' + channel + '?number=' + num, function() {
            location.reload();
          });
        });
        $('#drumrack-preset-modal-cancel').on('click', function() {
          $('#drumrack-preset-modal').fadeOut(100);
        });
      });
    });

    // --- Save Preset Handler ---
    $('#drumrack-save-preset').on('click', function() {
      $('#drumrack-preset-modal-content').html('<div>Loading presets...</div>');
      $('#drumrack-preset-modal').fadeIn(100);
      $.get('/api/v1/getPresets/' + channel, function(data) {
        if(typeof data === 'string') data = JSON.parse(data);
        let html = `<h3>Save Preset</h3><div>Current preset: <b>${data.activePresetNumber}: ${data.presets[data.activePresetNumber].name}</b></div>`;
        html += `<div>Patch Number: <input id='drumrack-save-num' type='number' min='0' max='9' value='${data.presets.length}' style='width:3em;'></div>`;
        html += `<div>Patch Name: <input id='drumrack-save-name' type='text' placeholder='Enter Patch Name'></div>`;
        html += `<button id='drumrack-save-confirm'>Save</button> <button id='drumrack-preset-modal-cancel'>Cancel</button>`;
        $('#drumrack-preset-modal-content').html(html);
        $('#drumrack-save-num').on('input', function() {
          const v = parseInt($(this).val());
          if(v < data.presets.length && v >= 0) {
            $('#drumrack-save-confirm').css('background','red').text('Overwrite Preset ?!');
            $('#drumrack-save-name').val(data.presets[v].name);
          } else {
            $('#drumrack-save-confirm').css('background','').text('Save');
            $('#drumrack-save-name').val('');
          }
        });
        $('#drumrack-save-confirm').on('click', function() {
          const num = $('#drumrack-save-num').val();
          const name = $('#drumrack-save-name').val();
          if(num < 0 || num > 9) {
            alert('Wrong patch number, must be 0-9');
            return;
          }
          $('#drumrack-preset-modal-content').html('Saving...');
          $.get('/api/v1/savePreset/' + channel + '?number=' + num + '&name=' + encodeURIComponent(name), function() {
            location.reload();
          });
        });
        $('#drumrack-preset-modal-cancel').on('click', function() {
          $('#drumrack-preset-modal').fadeOut(100);
        });
      });
    });

    // --- Load/Save Preset Button Handlers (icon buttons) ---
    $('#drumrack-preset-load').on('click', function() {
      $('#drumrack-preset-modal-content').html('<div>Loading presets...</div>');
      $('#drumrack-preset-modal').fadeIn(100);
      $.get('/api/v1/getPresets/' + channel, function(data) {
        if(typeof data === 'string') data = JSON.parse(data);
        let html = `<h3>Load Preset</h3><ul style='list-style:none;padding:0;'>`;
        data.presets.forEach(p => {
          html += `<li><button class='drumrack-preset-load-btn' data-num='${p.number}'>${p.number}: ${p.name}${data.activePresetNumber==p.number ? ' <b>(active)</b>' : ''}</button></li>`;
        });
        html += '</ul><button id="drumrack-preset-modal-cancel">Cancel</button>';
        $('#drumrack-preset-modal-content').html(html);
        $('.drumrack-preset-load-btn').on('click', function() {
          const num = $(this).data('num');
          $('#drumrack-preset-modal-content').html('Loading...');
          $.get('/api/v1/loadPreset/' + channel + '?number=' + num, function() {
            location.reload();
          });
        });
        $('#drumrack-preset-modal-cancel').on('click', function() {
          $('#drumrack-preset-modal').fadeOut(100);
        });
      });
    });

    $('#drumrack-preset-save').on('click', function() {
      $('#drumrack-preset-modal-content').html('<div>Loading presets...</div>');
      $('#drumrack-preset-modal').fadeIn(100);
      $.get('/api/v1/getPresets/' + channel, function(data) {
        if(typeof data === 'string') data = JSON.parse(data);
        let html = `<h3>Save Preset</h3><div>Current preset: <b>${data.activePresetNumber}: ${data.presets[data.activePresetNumber].name}</b></div>`;
        html += `<div>Patch Number: <input id='drumrack-save-num' type='number' min='0' max='9' value='${data.presets.length}' style='width:3em;'></div>`;
        html += `<div>Patch Name: <input id='drumrack-save-name' type='text' placeholder='Enter Patch Name'></div>`;
        html += `<button id='drumrack-save-confirm'>Save</button> <button id='drumrack-preset-modal-cancel'>Cancel</button>`;
        $('#drumrack-preset-modal-content').html(html);
        $('#drumrack-save-num').on('input', function() {
          const v = parseInt($(this).val());
          if(v < data.presets.length && v >= 0) {
            $('#drumrack-save-confirm').css('background','red').text('Overwrite Preset ?!');
            $('#drumrack-save-name').val(data.presets[v].name);
          } else {
            $('#drumrack-save-confirm').css('background','').text('Save');
            $('#drumrack-save-name').val('');
          }
        });
        $('#drumrack-save-confirm').on('click', function() {
          const num = $('#drumrack-save-num').val();
          const name = $('#drumrack-save-name').val();
          if(num < 0 || num > 9) {
            alert('Wrong patch number, must be 0-9');
            return;
          }
          $('#drumrack-preset-modal-content').html('Saving...');
          $.get('/api/v1/savePreset/' + channel + '?number=' + num + '&name=' + encodeURIComponent(name), function() {
            location.reload();
          });
        });
        $('#drumrack-preset-modal-cancel').on('click', function() {
          $('#drumrack-preset-modal').fadeOut(100);
        });
      });
    });

    // Collapsible groups
    $('.drumrack-group-header').on('click', function(e) {
        // Prevent toggling if help icon was clicked
        if ($(e.target).closest('.drumrack-group-help').length > 0) return;
        const group = $(this).closest('.drumrack-group');
        group.toggleClass('collapsed');
        group.find('.drumrack-params').slideToggle(150);
        $(this).find('.toggle').toggleClass('open');
    });
    // All groups start collapsed by default
    $('.drumrack-group').addClass('collapsed').find('.drumrack-params').hide();

    // Handle parameter changes
    function setParam(id, value) {
        $.get(`${SET_PARAM_URL}?id=${encodeURIComponent(id)}&current=${encodeURIComponent(value)}`);
    }
    // Debounced for sliders
    const debouncedSetParam = debounce(setParam, 120);

    // Slider/checkbox events
    $('#drumrack-ui').on('input change', 'input[type=range]', function() {
        const id = $(this).data('id');
        const val = $(this).val();
        $(`#val-${id}`).text(val);
        debouncedSetParam(id, val);
    });
    $('#drumrack-ui').on('change', 'input[type=checkbox]', function() {
        const id = $(this).data('id');
        const val = $(this).is(':checked') ? 1 : 0;
        setParam(id, val);
    });
    // Trigger button events
    $('#drumrack-ui').on('click', '.trigger-btn', function() {
        const id = $(this).data('id');
        setParam(id, 1);
        setTimeout(() => setParam(id, 0), 100); // Reset trigger after short delay
    });
    // Add event for double-click to edit value
    $('#drumrack-ui').on('dblclick', '.param-value.param-editable', function() {
        const id = $(this).data('id');
        const min = $(`#${id}`).attr('min');
        const max = $(`#${id}`).attr('max');
        const step = $(`#${id}`).attr('step') || 1;
        const current = $(this).text();
        // Replace span with input, right-aligned
        const input = $(`<input type="number" class="param-inline-input" min="${min}" max="${max}" step="${step}" value="${current}" style="width:60px;text-align:right;">`);
        $(this).replaceWith(input);
        input.focus();
        input.select();
    });
    // Handle blur/enter for inline input
    $('#drumrack-ui').on('blur', '.param-inline-input', function() {
        finishInlineEdit($(this));
    });
    $('#drumrack-ui').on('keydown', '.param-inline-input', function(e) {
        if (e.key === 'Enter') {
            finishInlineEdit($(this));
        } else if (e.key === 'Escape') {
            // Cancel edit
            const id = $(this).closest('.drumrack-param').find('input[type=range]').data('id');
            const val = $(`#${id}`).val();
            $(this).replaceWith(`<span class="param-value param-editable" id="val-${id}" data-id="${id}">${val}</span>`);
        }
    });
    function finishInlineEdit($input) {
        const id = $input.closest('.drumrack-param').find('input[type=range]').data('id');
        let val = $input.val();
        const min = $input.attr('min');
        const max = $input.attr('max');
        if (val < min) val = min;
        if (val > max) val = max;
        $(`#${id}`).val(val);
        $input.replaceWith(`<span class="param-value param-editable" id="val-${id}" data-id="${id}">${val}</span>`);
        debouncedSetParam(id, val);
    }

    // --- Automation Mapping Context Menu ---
    if (!$('#drumrack-automation-modal').length) {
      $('body').append(`
        <div id="drumrack-automation-modal" style="display:none;position:fixed;z-index:2000;left:0;top:0;width:100vw;height:100vh;background:rgba(0,0,0,0.15);align-items:center;justify-content:center;">
          <div id="drumrack-automation-modal-content" style="background:#fff;padding:1.5em 2em 1em 2em;border-radius:10px;min-width:260px;max-width:90vw;"></div>
        </div>
      `);
    }

    // Helper to get ioCaps (CV/TRIG sources)
    function getIOCaps(cb) {
      if (window.ioCaps) return cb(window.ioCaps);
      $.get('/api/v1/getIOCaps', function(data) {
        if(typeof data === 'string') data = JSON.parse(data);
        window.ioCaps = data;
        cb(data);
      });
    }

    // Listen for shift-click on parameter controls
    $('#drumrack-ui').off('click.drumrack-automation').on('click.drumrack-automation', '.drumrack-param input, .drumrack-param button, .drumrack-param label', function(e) {
      if (!e.shiftKey) return;
      e.preventDefault();
      e.stopPropagation();
      const $param = $(this).closest('.drumrack-param');
      const id = $param.find('input,button').data('id');
      if (!id) return;
      // Find param definition for name/type
      function findParamById(params, pid) {
        for (const p of params) {
          if (p.type === 'group') {
            const found = findParamById(p.params, pid);
            if (found) return found;
          } else if (p.id === pid) {
            return p;
          }
        }
        return null;
      }
      const paramDefObj = findParamById(paramVals.params, id);
      const paramName = paramDefObj ? paramDefObj.name : id;
      const paramType = paramDefObj ? paramDefObj.type : null;
      getIOCaps(function(ioCaps) {
        // Get current mapping for this param (if any)
        let currentTrig = null, currentCV = null;
        if ($param.data('trig') !== undefined) currentTrig = $param.data('trig');
        if ($param.data('cv') !== undefined) currentCV = $param.data('cv');
        let html = `<div style='font-size:1em;min-width:100px;max-width:180px;'>`;
        html += `<div style='font-weight:bold;margin-bottom:0.2em;'>${paramName}</div>`;
        if(paramType === 'bool') {
          html += `<div><select id='drumrack-map-trig' style='width:100%;margin-bottom:0.3em;'>`;
          html += `<option value='-1'>TRIG: None</option>`;
          (ioCaps.t||[]).forEach((trig, i) => { html += `<option value='${i}'>TRIG: ${trig}</option>`; });
          html += `</select></div>`;
        } else {
          html += `<div><select id='drumrack-map-cv' style='width:100%;margin-bottom:0.3em;'>`;
          html += `<option value='-1'>CV: None</option>`;
          (ioCaps.cv||[]).forEach((cv, i) => { html += `<option value='${i}'>CV: ${cv}</option>`; });
          html += `</select></div>`;
        }
        html += `</div>`;
        $('#drumrack-automation-modal-content').html(html);
        // Set select to current mapping
        if(paramType === 'bool' && currentTrig !== null && currentTrig !== undefined) {
          $('#drumrack-map-trig').val(currentTrig);
        } else if(paramType !== 'bool' && currentCV !== null && currentCV !== undefined) {
          $('#drumrack-map-cv').val(currentCV);
        }
        // Position modal over the clicked element
        const offset = $param.offset();
        const paramHeight = $param.outerHeight();
        const modal = $('#drumrack-automation-modal');
        modal.css({display:'flex', position:'absolute', left: offset.left + 'px', top: (offset.top + paramHeight + 4) + 'px', width: 'auto', height: 'auto', background: 'none', 'align-items':'flex-start', 'justify-content':'flex-start'});
        $('#drumrack-automation-modal-content').css({boxShadow:'0 2px 16px rgba(0,0,0,0.18)',padding:'0.4em 0.7em 0.4em 0.7em', minWidth:'unset', maxWidth:'unset'});
        // Keyboard navigation
        const selectId = paramType === 'bool' ? '#drumrack-map-trig' : '#drumrack-map-cv';
        $(selectId).focus();
        $(selectId).on('keydown', function(ev) {
          if(ev.key === 'Enter') {
            $('#drumrack-automation-modal').fadeOut(100);
          } else if(ev.key.toLowerCase() === 'q') {
            $('#drumrack-map-cancel').click();
          } else if(ev.key === 'ArrowDown' || ev.key === 'ArrowUp') {
            // Fast edit with cursor keys
            const $sel = $(this);
            let idx = $sel.prop('selectedIndex');
            if(ev.key === 'ArrowDown') {
              $sel.prop('selectedIndex', Math.min(idx+1, $sel.find('option').length-1));
              $sel.trigger('change');
              ev.preventDefault();
            } else if(ev.key === 'ArrowUp') {
              $sel.prop('selectedIndex', Math.max(idx-1, 0));
              $sel.trigger('change');
              ev.preventDefault();
            }
          }
        });
        // Preselect current mapping if available (from paramVals)
        function findCurrentMapping(params, pid) {
          for (const p of params) {
            if (p.type === 'group') {
              const found = findCurrentMapping(p.params, pid);
              if (found) return found;
            } else if (p.id === pid) {
              return p;
            }
          }
          return null;
        }
        const paramValObj = findCurrentMapping(paramVals.params, id);
        if(paramType === 'bool' && paramValObj && typeof paramValObj.trig !== 'undefined') {
          $('#drumrack-map-trig').val(paramValObj.trig);
        } else if(paramType !== 'bool' && paramValObj && typeof paramValObj.cv !== 'undefined') {
          $('#drumrack-map-cv').val(paramValObj.cv);
        }
        // Remove apply/cancel handlers, close modal only on Enter, not on change
        if(paramType === 'bool') {
          $('#drumrack-map-trig').on('change', function() {
            const trig = $(this).val();
            $.get(`/api/v1/setPluginParamTRIG/${channel}?id=${encodeURIComponent(id)}&trig=${trig}`);
            if(paramValObj) paramValObj.trig = parseInt(trig);
            // Do not close modal here
          });
          $('#drumrack-map-trig').on('keydown', function(ev) {
            if(ev.key === 'Enter') {
              $('#drumrack-automation-modal').fadeOut(100);
            }
          });
        } else {
          $('#drumrack-map-cv').on('change', function() {
            const cv = $(this).val();
            $.get(`/api/v1/setPluginParamCV/${channel}?id=${encodeURIComponent(id)}&cv=${cv}`);
            if(paramValObj) paramValObj.cv = parseInt(cv);
            // Do not close modal here
          });
          $('#drumrack-map-cv').on('keydown', function(ev) {
            if(ev.key === 'Enter') {
              $('#drumrack-automation-modal').fadeOut(100);
            }
          });
        }
        // --- Hide modal on outside click ---
        setTimeout(function() {
          $(document).on('mousedown.drumrack-automation-modal', function(e) {
            if (!$(e.target).closest('#drumrack-automation-modal-content').length) {
              $('#drumrack-automation-modal').fadeOut(100);
              $(document).off('mousedown.drumrack-automation-modal');
            }
          });
        }, 0);
        // --- Remove handler when modal closes (for safety) ---
        $('#drumrack-automation-modal').on('fadeOut', function() {
          $(document).off('mousedown.drumrack-automation-modal');
        });
      });
    });

    // --- Live mapping indicator update ---
    async function updateMappingIndicators() {
        const paramValsLive = await fetchJSON(PARAMS_URL);
        function updateIndicators(params) {
            params.forEach(p => {
                if (p.type === 'group') updateIndicators(p.params);
                else {
                    let cv = typeof p.cv !== 'undefined' ? p.cv : -1;
                    let trig = typeof p.trig !== 'undefined' ? p.trig : -1;
                    const active = (cv !== -1) || (trig !== -1);
                    const el = document.getElementById('mapping-indicator-' + p.id);
                    if (el) el.style.display = active ? 'inline-block' : 'none';
                }
            });
        }
        updateIndicators(paramValsLive.params);
    }

    // Optionally, poll for mapping changes every 2s
    setInterval(updateMappingIndicators, 2000);

    // --- Help Modal ---
    if (!$('#drumrack-help-modal').length) {
      $('body').append(`
        <div id="drumrack-help-modal" style="display:none;position:fixed;z-index:3000;left:0;top:0;width:100vw;height:100vh;background:rgba(0,0,0,0.18);align-items:center;justify-content:center;">
          <div id="drumrack-help-modal-content" style="background:#fff;padding:2em 2.5em 1.5em 2.5em;border-radius:12px;min-width:320px;max-width:96vw;box-shadow:0 4px 24px rgba(0,0,0,0.10),0 1px 4px rgba(0,0,0,0.04);font-size:1.07em;color:#222;display:flex;flex-direction:column;align-items:stretch;gap:1.1em;">
            <h3 style="margin:0 0 1em 0;font-size:1.18em;font-weight:600;color:#2a3a4d;text-align:center;">DrumRack UI Help</h3>
            <ul style="margin:0 0 1.1em 0;padding:0;list-style:none;display:flex;flex-direction:column;gap:0.7em;">
              <li><b>Adjust parameters</b> using sliders, checkboxes, and trigger buttons in each group.</li>
              <li><b>Double-click</b> a value to type in a precise number.</li>
              <li><b>Shift-click</b> any parameter to assign CV or TRIG mapping (shows a mapping menu).</li>
              <li><b>Click group headers</b> to expand or collapse parameter groups.</li>
              <li><b>Trigger</b> buttons send a one-shot trigger to the sound.</li>
              <li><b>Use the sidebar</b> to <b>load</b> or <b>save</b> presets, toggle mapping display, or open this help.</li>
              <li><b>Toggle mapping display</b> (sidebar switch) to show/hide which parameters are mapped to CV or TRIG inputs.</li>
            </ul>
            <button id="drumrack-help-modal-close" style="margin:0.3em auto 0 auto;padding:0.55em 1.3em;border-radius:7px;border:none;background:#bfc7d1;color:#222;font-size:1em;font-weight:500;cursor:pointer;transition:background 0.2s;box-shadow:0 1px 4px rgba(0,0,0,0.04);min-width:110px;line-height:1.4;">Close</button>
          </div>
        </div>
      `);
    }
    $('#drumrack-help').off('click').on('click', function() {
      $('#drumrack-help-modal').fadeIn(120);
    });
    $(document).off('click.drumrack-help-modal').on('click.drumrack-help-modal', function(e) {
      if ($(e.target).is('#drumrack-help-modal, #drumrack-help-modal-close')) {
        $('#drumrack-help-modal').fadeOut(120);
      }
    });

    // Mapping toggle logic
    let showMappings = false;
    // Set initial icon state based on showMappings
    $('#drumrack-mapping-toggle-img').attr('src', showMappings ? 'img/toggle_on.svg' : 'img/toggle_off.svg');
    $('#drumrack-mapping-toggle').off('click').on('click', function() {
        showMappings = !showMappings;
        $('#drumrack-mapping-toggle-img').attr('src', showMappings ? 'img/toggle_on.svg' : 'img/toggle_off.svg');
        updateMappingTextDisplay();
    });

    function getMappingText(param) {
        if (typeof param.cv !== 'undefined' && param.cv !== -1 && window.ioCaps && window.ioCaps.cv) {
            return 'CV: ' + (window.ioCaps.cv[param.cv] || param.cv);
        }
        if (typeof param.trig !== 'undefined' && param.trig !== -1 && window.ioCaps && window.ioCaps.t) {
            return 'TRIG: ' + (window.ioCaps.t[param.trig] || param.trig);
        }
        return '';
    }

    function updateMappingTextDisplay() {
        // For each mapping-indicator, add/remove mapping text
        function update(params) {
            params.forEach(p => {
                if (p.type === 'group') update(p.params);
                else {
                    const el = document.getElementById('mapping-indicator-' + p.id);
                    if (el) {
                        let txtEl = el.nextSibling;
                        if (txtEl && (!txtEl.className || txtEl.className !== 'mapping-text')) txtEl = null;
                        if (showMappings && ((typeof p.cv !== 'undefined' && p.cv !== -1) || (typeof p.trig !== 'undefined' && p.trig !== -1))) {
                            const txt = getMappingText(p);
                            if (!txtEl) {
                                const span = document.createElement('span');
                                span.className = 'mapping-text';
                                span.style.marginLeft = '0.3em';
                                span.style.color = '#b71c1c';
                                span.style.fontSize = '0.97em';
                                span.textContent = txt;
                                el.parentNode.insertBefore(span, el.nextSibling);
                            } else {
                                txtEl.textContent = txt;
                            }
                        } else if (txtEl) {
                            txtEl.remove();
                        }
                    }
                }
            });
        }
        update(paramVals.params);
    }

    // Ensure ioCaps is loaded before showing mapping text
    function ensureIOCapsAndUpdate() {
        if (window.ioCaps) {
            updateMappingTextDisplay();
        } else {
            getIOCaps(function() {
                updateMappingTextDisplay();
            });
        }
    }

    // Update mapping text when mapping changes
    setInterval(function() {
        if (showMappings) ensureIOCapsAndUpdate();
    }, 2000);
    // Also update on toggle
    $('#drumrack-mapping-toggle').on('click', ensureIOCapsAndUpdate);
    // Initial update if toggled on page load
    if (showMappings) ensureIOCapsAndUpdate;

    // --- Model Group Help Modal Logic ---
    if (!$('#drumrack-model-help-modal').length) {
        $('body').append(`
            <div id="drumrack-model-help-modal" style="display:none;position:fixed;z-index:3100;left:0;top:0;width:100vw;height:100vh;background:rgba(0,0,0,0.18);align-items:center;justify-content:center;">
                <div id="drumrack-model-help-modal-content" style="background:#fff;padding:2em 2.5em 1.5em 2.5em;border-radius:12px;min-width:320px;max-width:96vw;box-shadow:0 4px 24px rgba(0,0,0,0.10),0 1px 4px rgba(0,0,0,0.04);font-size:1.07em;color:#222;display:flex;flex-direction:column;align-items:stretch;gap:1.1em;"></div>
            </div>
        `);
    }
    // Model help content (add/adjust as needed)
    const modelHelpContent = {
        'ab_group': {
            title: 'Analog Bass Drum',
            body: `<div style="margin-bottom:1em;">
                <b>Analog Bass Drum</b> is a classic analog-style kick drum synthesizer. It features a punchy, deep sound with flexible shaping.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output of the bass drum.</li>
                  <li><b>Trigger</b>: Triggers the drum sound.</li>
                  <li><b>Level</b>: Output volume (squared response for finer control at low levels).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Accent</b>: Accent amount for punchier hits.</li>
                  <li><b>Pitch (F0)</b>: Base pitch of the drum (Hz, mapped from 0.0001 to 0.01).</li>
                  <li><b>Tone</b>: Brightness of the drum sound.</li>
                  <li><b>Decay</b>: Length of the drum sound.</li>
                  <li><b>Attack FM</b>: Amount of frequency modulation on attack (adds click/punch).</li>
                  <li><b>Sustain FM</b>: Amount of frequency modulation on sustain (adds body/weight).</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. Accent and FM controls allow for a wide range of classic and modern kick sounds.
              </div>`
        },
        'as_group': {
            title: 'Analog Snare Drum',
            body: `<div style="margin-bottom:1em;">
                <b>Analog Snare Drum</b> provides a classic snare sound with analog-style noise and tone shaping.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the snare output.</li>
                  <li><b>Trigger</b>: Triggers the snare sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Accent</b>: Accent amount for snappier hits.</li>
                  <li><b>Pitch (F0)</b>: Base pitch of the snare (Hz, mapped from 0.001 to 0.01).</li>
                  <li><b>Tone</b>: Brightness of the snare body.</li>
                  <li><b>Decay</b>: Length of the snare sound.</li>
                  <li><b>Noise</b>: Amount of noise in the snare (controls the 'snappy' part).</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. Accent and noise controls allow for a range of classic and modern snare timbres.
              </div>`
        },
        'db_group': {
            title: 'Digital Bass Drum',
            body: `<div style="margin-bottom:1em;">
                <b>Digital Bass Drum</b> is a modern kick drum with digital FM, dirt, and envelope shaping.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Trigger</b>: Triggers the drum sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Accent</b>: Accent amount for punchier hits.</li>
                  <li><b>Pitch (F0)</b>: Base pitch of the drum (Hz, mapped from 0.0005 to 0.01).</li>
                  <li><b>Tone</b>: Brightness of the drum sound.</li>
                  <li><b>Decay</b>: Length of the drum sound.</li>
                  <li><b>Dirt</b>: Adds digital distortion and grit.</li>
                  <li><b>FM Env</b>: Amount of FM envelope applied.</li>
                  <li><b>FM Decay</b>: Decay time of the FM envelope.</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. FM and dirt controls allow for aggressive, modern kick sounds.
              </div>`
        },
        'ds_group': {
            title: 'Digital Snare Drum',
            body: `<div style="margin-bottom:1em;">
                <b>Digital Snare Drum</b> is a modern snare with FM, tone, and noise shaping.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Trigger</b>: Triggers the snare sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Accent</b>: Accent amount for snappier hits.</li>
                  <li><b>Pitch (F0)</b>: Base pitch of the snare (Hz, mapped from 0.0008 to 0.01).</li>
                  <li><b>FM Amount</b>: Amount of frequency modulation applied.</li>
                  <li><b>Decay</b>: Length of the snare sound.</li>
                  <li><b>Noise</b>: Amount of noise in the snare (controls the 'snappy' part).</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. FM and noise controls allow for a range of digital snare timbres.
              </div>`
        },
        'hh1_group': {
            title: 'HiHat 1',
            body: `<div style="margin-bottom:1em;">
                <b>HiHat 1</b> is a closed or open hihat with analog-style synthesis.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Trigger</b>: Triggers the hihat sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Accent</b>: Accent amount for sharper hits.</li>
                  <li><b>Pitch (F0)</b>: Base pitch of the hihat (Hz, mapped from 0.0005 to 0.1).</li>
                  <li><b>Tone</b>: Brightness of the hihat sound.</li>
                  <li><b>Decay</b>: Length of the hihat sound.</li>
                  <li><b>Noise</b>: Amount of noise in the hihat (controls metallic character).</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. Accent and noise controls allow for a range of hihat timbres.
              </div>`
        },
        'hh2_group': {
            title: 'HiHat 2',
            body: `<div style="margin-bottom:1em;">
                <b>HiHat 2</b> is a second hihat channel for layering or variation.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Trigger</b>: Triggers the hihat sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Accent</b>: Accent amount for sharper hits.</li>
                  <li><b>Pitch (F0)</b>: Base pitch of the hihat (Hz, mapped from 0.00001 to 0.1).</li>
                  <li><b>Tone</b>: Brightness of the hihat sound.</li>
                  <li><b>Decay</b>: Length of the hihat sound.</li>
                  <li><b>Noise</b>: Amount of noise in the hihat (controls metallic character).</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. Accent and noise controls allow for a range of hihat timbres.
              </div>`
        },
        'rs_group': {
            title: 'Rimshot',
            body: `<div style="margin-bottom:1em;">
                <b>Rimshot</b> is a percussive rimshot with pitch, decay, noise, and accent controls.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Trigger</b>: Triggers the rimshot sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Accent</b>: Accent amount for punchier hits.</li>
                  <li><b>Pitch (F0)</b>: Base pitch of the rimshot (Hz, mapped from 70 to 350).</li>
                  <li><b>Tone</b>: Brightness of the rimshot sound.</li>
                  <li><b>Decay</b>: Length of the rimshot sound.</li>
                  <li><b>Noise</b>: Amount of noise in the rimshot (adds realism).</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. Accent and noise controls allow for a range of rimshot timbres.
              </div>`
        },
        'cl_group': {
            title: 'Clap',
            body: `<div style="margin-bottom:1em;">
                <b>Clap</b> is a classic handclap with pitch, resonance, decay, and transient controls.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Trigger</b>: Triggers the clap sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Pitch 1/2</b>: Base pitch of the two clap oscillators (Hz, mapped from 350–4000 and 300–3000).</li>
                  <li><b>Resonance 1/2</b>: Resonance of the two oscillators.</li>
                  <li><b>Decay 1/2</b>: Decay time of the two oscillators.</li>
                  <li><b>Attack</b>: Attack time of the clap.</li>
                  <li><b>Scale</b>: Scaling factor for the transient.</li>
                  <li><b>Transient</b>: Number of transients (0–15) for clap realism.</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. Pitch, resonance, and transient controls allow for a range of clap timbres.
              </div>`
        },
        'fmb_group': {
            title: 'FM Bass Drum',
            body: `<div style="margin-bottom:1em;">
                <b>FM Bass Drum (FMKick)</b> is a flexible bass drum based on two-operator FM synthesis.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Trigger</b>: Triggers the drum sound.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Use Ratio Mode</b>: Sets modulator frequency as a ratio of carrier frequency.</li>
                  <li><b>Mod Env Sync</b>: Modulator envelope follows carrier envelope for tighter attacks.</li>
                  <li><b>Base Frequency (f_b)</b>: Carrier oscillator base frequency (Hz, 20–200).</li>
                  <li><b>Decay Base (d_b)</b>: Decay time for amplitude envelope (0.001–1s).</li>
                  <li><b>Modulator Frequency (f_m)</b>: Modulator oscillator frequency (Hz, 40–2000).</li>
                  <li><b>Mod Ratio Index</b>: Ratio of modulator to carrier frequency (0–63, used if Ratio Mode is on).</li>
                  <li><b>Mod Index (I)</b>: FM modulation depth (0–10).</li>
                  <li><b>Decay Mod (d_m)</b>: Decay time for modulator envelope (0.001–0.5s).</li>
                  <li><b>Feedback (b_m)</b>: Feedback amount for modulator (0–16, adds metallic/dirty character).</li>
                  <li><b>Amp Freq (A_f)</b>: Amount of pitch envelope applied to carrier (0–1000).</li>
                  <li><b>Decay Freq (d_f)</b>: Decay time for pitch envelope (0.001–0.1s).</li>
                </ul>
                <b>General:</b> Use triggers to play, and adjust FX sends for delay/reverb. FM controls allow for a wide range of classic and modern kick sounds.<br><br>
                <b>FM Synthesis Structure:</b><br>
                <div style='text-align:center;margin:1em 0;'>
                  <svg width='260' height='110' viewBox='0 0 260 110'>
                    <rect x='10' y='40' width='60' height='30' rx='8' fill='#e0e7ef' stroke='#2a3a4d' stroke-width='2'/>
                    <text x='40' y='60' text-anchor='middle' font-size='13' fill='#2a3a4d'>Modulator</text>
                    <rect x='110' y='40' width='60' height='30' rx='8' fill='#e0e7ef' stroke='#2a3a4d' stroke-width='2'/>
                    <text x='140' y='60' text-anchor='middle' font-size='13' fill='#2a3a4d'>Carrier</text>
                    <rect x='210' y='40' width='40' height='30' rx='8' fill='#e0e7ef' stroke='#2a3a4d' stroke-width='2'/>
                    <text x='230' y='60' text-anchor='middle' font-size='13' fill='#2a3a4d'>Out</text>
                    <line x1='70' y1='55' x2='110' y2='55' stroke='#2a3a4d' stroke-width='2' marker-end='url(#arrow)'/>
                    <line x1='170' y1='55' x2='210' y2='55' stroke='#2a3a4d' stroke-width='2' marker-end='url(#arrow)'/>
                    <ellipse cx='40' cy='25' rx='18' ry='10' fill='#f7fafd' stroke='#2a3a4d' stroke-width='1.2'/>
                    <text x='40' y='29' text-anchor='middle' font-size='11' fill='#2a3a4d'>Mod Env</text>
                    <ellipse cx='140' cy='25' rx='18' ry='10' fill='#f7fafd' stroke='#2a3a4d' stroke-width='1.2'/>
                    <text x='140' y='29' text-anchor='middle' font-size='11' fill='#2a3a4d'>Amp Env</text>
                    <ellipse cx='140' cy='90' rx='18' ry='10' fill='#f7fafd' stroke='#2a3a4d' stroke-width='1.2'/>
                    <text x='140' y='94' text-anchor='middle' font-size='11' fill='#2a3a4d'>Freq Env</text>
                    <line x1='40' y1='35' x2='40' y2='40' stroke='#2a3a4d' stroke-width='1.2'/>
                    <line x1='140' y1='35' x2='140' y2='40' stroke='#2a3a4d' stroke-width='1.2'/>
                    <line x1='140' y1='80' x2='140' y2='70' stroke='#2a3a4d' stroke-width='1.2'/>
                    <defs>
                      <marker id='arrow' markerWidth='8' markerHeight='8' refX='7' refY='4' orient='auto' markerUnits='strokeWidth'>
                        <path d='M0,0 L8,4 L0,8 L2,4 z' fill='#2a3a4d'/>
                      </marker>
                    </defs>
                  </svg>
                </div>
                <span style='font-size:0.97em;color:#3a4a5d;'>The modulator oscillator (left) modulates the frequency of the carrier oscillator (center). Each has its own envelope. The carrier output is the drum sound. Pitch envelope (Freq Env) shapes the attack.</span>
              </div>`
        },
        's1_group': {
            title: 'Rompler 1',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 1</b> plays back user samples with flexible slicing, pitch, and filter controls.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Gate</b>: Triggers sample playback.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Speed</b>: Playback speed (0–2x, CV-controllable).</li>
                  <li><b>Pitch</b>: Pitch offset (CV-controllable).</li>
                  <li><b>Bank/Slice</b>: Selects sample bank and slice.</li>
                  <li><b>Start/End</b>: Start and end position within the slice (relative).</li>
                  <li><b>Loop/Loop PiPo</b>: Enables looping and ping-pong looping.</li>
                  <li><b>Loop Marker</b>: Position of loop marker (relative).</li>
                  <li><b>Attack/Decay</b>: Envelope attack and decay times.</li>
                  <li><b>EG→FM</b>: Envelope to FM amount.</li>
                  <li><b>Bit Reduction</b>: Reduces bit depth for lo-fi effects.</li>
                  <li><b>Filter Type</b>: Selects filter type (LPF, HPF, BPF, etc).</li>
                  <li><b>Cutoff/Resonance</b>: Filter cutoff and resonance.</li>
                </ul>
                <b>General:</b> Use Gate to trigger, and adjust slice/bank for sample selection. All parameters can be modulated for creative effects.
              </div>`
        },
        's2_group': {
            title: 'Rompler 2',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 2</b> is identical to Rompler 1, with independent parameters and sample selection. See Rompler 1 for parameter details.
              </div>`
        },
        's3_group': {
            title: 'Rompler 3',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 3</b> is identical to Rompler 1, with independent parameters and sample selection. See Rompler 1 for parameter details.
              </div>`
        },
        's4_group': {
            title: 'Rompler 4',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 4</b> is identical to Rompler 1, with independent parameters and sample selection. See Rompler 1 for parameter details.
              </div>`
        },
        'fx1_group': {
            title: 'Delay FX',
            body: `<div style="margin-bottom:1em;">
                <b>Delay FX</b> is a flexible stereo delay with feedback, filtering, and sync options.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Time (ms)</b>: Delay time in milliseconds (CV-controllable).</li>
                  <li><b>Sync</b>: Syncs delay time to external trigger.</li>
                  <li><b>Freeze</b>: Freezes the delay buffer.</li>
                  <li><b>Tape/Digital</b>: Switches between tape-style and digital delay.</li>
                  <li><b>Stereo Width</b>: Controls stereo spread of delay.</li>
                  <li><b>Reverb Send</b>: Amount sent from delay to reverb.</li>
                  <li><b>Feedback</b>: Amount of feedback (CV-controllable).</li>
                  <li><b>Base/Width</b>: Highpass and lowpass filter cutoff frequencies.</li>
                  <li><b>Amount</b>: Output level of the delay effect.</li>
                </ul>
                <b>General:</b> Use feedback and filtering for creative delay effects. Sync and freeze allow for rhythmic and experimental textures.
              </div>`
        },
        'fx2_group': {
            title: 'Reverb FX',
            body: `<div style="margin-bottom:1em;">
                <b>Reverb FX</b> is a stereo reverb with time, amount, and lowpass filter controls.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Time</b>: Reverb decay time (CV-controllable).</li>
                  <li><b>Amount</b>: Output level of the reverb effect (CV-controllable).</li>
                  <li><b>Lowpass</b>: Lowpass filter cutoff for the reverb tail (CV-controllable).</li>
                </ul>
                <b>General:</b> Use reverb to add space and depth to the drum mix.
              </div>`
        },
        'c_group': {
            title: 'Compressor',
            body: `<div style="margin-bottom:1em;">
                <b>Compressor</b> is a stereo bus compressor for the drum mix.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Threshold (dB)</b>: Level above which compression is applied (-80 to 0 dB).</li>
                  <li><b>Attack</b>: Attack time in ms (0.3–30 ms).</li>
                  <li><b>Release</b>: Release time in ms (40–2000 ms).</li>
                  <li><b>Ratio</b>: Compression ratio (0.0001–1.25).</li>
                  <li><b>Sidechain LPF</b>: Enables lowpass filtering of the sidechain signal.</li>
                  <li><b>Mix</b>: Dry/wet mix for parallel compression.</li>
                  <li><b>Gain</b>: Makeup gain in dB (0–60 dB).</li>
                  <li><b>Delay/Rev Level</b>: Amount of delay/reverb sent to compressor sidechain.</li>
                </ul>
                <b>General:</b> Use compressor to control dynamics and glue the drum mix together.
              </div>`
        },
        'sum': {
            title: 'Overall Mix',
            body: `<div style="margin-bottom:1em;">
                <b>Overall Mix</b> controls the final output level and mute for the drum rack.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the entire drum rack output.</li>
                  <li><b>Level</b>: Final output level (squared response).</li>
                </ul>
                <b>General:</b> Use mute and level to control the overall output of the drum rack.
              </div>`
        }
    };

    // --- Model Group Help Modal Logic (old style) ---
    if (!$('#drumrack-model-help-modal-old').length) {
        $('body').append(`
            <div id="drumrack-model-help-modal-old" style="display:none;position:fixed;z-index:3100;left:0;top:0;width:100vw;height:100vh;background:rgba(0,0,0,0.18);align-items:center;justify-content:center;">
                <div id="drumrack-model-help-modal-content-old" style="background:#fff;padding:2em 2.5em 1.5em 2.5em;border-radius:12px;min-width:320px;max-width:96vw;box-shadow:0 4px 24px rgba(0,0,0,0.10),0 1px 4px rgba(0,0,0,0.04);font-size:1.07em;color:#222;display:flex;flex-direction:column;align-items:stretch;gap:1.1em;"></div>
            </div>
        `);
    }
    // Model help content (old style, for reference)
    const modelHelpContentOld = {
        'ab_group': {
            title: 'Analog Bass Drum',
            body: 'Classic analog bass drum synthesis with accent, pitch, tone, decay, and FM controls. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'as_group': {
            title: 'Analog Snare Drum',
            body: 'Analog snare with tone, decay, accent, and noise controls. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'db_group': {
            title: 'Digital Bass Drum',
            body: 'Digital bass drum with extra FM, dirt, and envelope shaping. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'ds_group': {
            title: 'Digital Snare Drum',
            body: 'Digital snare with FM, tone, decay, and noise controls. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'hh1_group': {
            title: 'HiHat 1',
            body: 'Closed or open hihat with pitch, tone, decay, and noise controls. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'hh2_group': {
            title: 'HiHat 2',
            body: 'Second hihat channel for layering or variation. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'rs_group': {
            title: 'Rimshot',
            body: 'Percussive rimshot with pitch, decay, noise, and accent controls. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'cl_group': {
            title: 'Clap',
            body: 'Classic clap with pitch, resonance, decay, and transient controls. Use triggers to play, and adjust FX sends for delay/reverb.'
        },
        'fmb_group': {
            title: 'FM Bass Drum',
            body: `<div style="margin-bottom:1em;">
                <b>FM Bass Drum (FMKick)</b> is a flexible bass drum based on two-operator FM synthesis.<br><br>
                <b>Parameters:</b><br>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Trigger</b>: Triggers the drum sound.</li>
                  <li><b>Mute</b>: Mutes the output.</li>
                  <li><b>Level</b>: Output volume.</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect.</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect.</li>
                  <li><b>Use Ratio Mode</b>: If enabled, sets modulator frequency as a ratio of the carrier frequency (classic FM drum sounds).</li>
                  <li><b>Mod Env Sync</b>: If enabled, modulator envelope follows the carrier envelope for tighter attacks.</li>
                  <li><b>Base Frequency (f_b)</b>: Carrier oscillator base frequency (pitch of the drum).</li>
                  <li><b>Decay Base (d_b)</b>: Decay time for the main amplitude envelope (how long the drum rings out).</li>
                  <li><b>Modulator Frequency (f_m)</b>: Frequency of the modulator oscillator (controls overtone content).</li>
                  <li><b>Mod Ratio Index</b>: Ratio of modulator to carrier frequency (used if Ratio Mode is on, 0–63).</li>
                  <li><b>Mod Index (I)</b>: FM modulation depth (amount of frequency modulation, controls brightness/attack).</li>
                  <li><b>Decay Mod (d_m)</b>: Decay time for the modulator envelope (how quickly the FM effect fades).</li>
                  <li><b>Feedback (b_m)</b>: Feedback amount for the modulator (adds metallic/dirty character).</li>
                  <li><b>Amp Freq (A_f)</b>: Amount of pitch envelope applied to the carrier (for punchy attacks).</li>
                  <li><b>Decay Freq (d_f)</b>: Decay time for the pitch envelope (how quickly the pitch drops).</li>
                </ul>
                <b>FM Synthesis Structure:</b><br>
                <div style='text-align:center;margin:1em 0;'>
                  <svg width='260' height='110' viewBox='0 0 260 110'>
                    <rect x='10' y='40' width='60' height='30' rx='8' fill='#e0e7ef' stroke='#2a3a4d' stroke-width='2'/>
                    <text x='40' y='60' text-anchor='middle' font-size='13' fill='#2a3a4d'>Modulator</text>
                    <rect x='110' y='40' width='60' height='30' rx='8' fill='#e0e7ef' stroke='#2a3a4d' stroke-width='2'/>
                    <text x='140' y='60' text-anchor='middle' font-size='13' fill='#2a3a4d'>Carrier</text>
                    <rect x='210' y='40' width='40' height='30' rx='8' fill='#e0e7ef' stroke='#2a3a4d' stroke-width='2'/>
                    <text x='230' y='60' text-anchor='middle' font-size='13' fill='#2a3a4d'>Out</text>
                    <line x1='70' y1='55' x2='110' y2='55' stroke='#2a3a4d' stroke-width='2' marker-end='url(#arrow)'/>
                    <line x1='170' y1='55' x2='210' y2='55' stroke='#2a3a4d' stroke-width='2' marker-end='url(#arrow)'/>
                    <ellipse cx='40' cy='25' rx='18' ry='10' fill='#f7fafd' stroke='#2a3a4d' stroke-width='1.2'/>
                    <text x='40' y='29' text-anchor='middle' font-size='11' fill='#2a3a4d'>Mod Env</text>
                    <ellipse cx='140' cy='25' rx='18' ry='10' fill='#f7fafd' stroke='#2a3a4d' stroke-width='1.2'/>
                    <text x='140' y='29' text-anchor='middle' font-size='11' fill='#2a3a4d'>Amp Env</text>
                    <ellipse cx='140' cy='90' rx='18' ry='10' fill='#f7fafd' stroke='#2a3a4d' stroke-width='1.2'/>
                    <text x='140' y='94' text-anchor='middle' font-size='11' fill='#2a3a4d'>Freq Env</text>
                    <line x1='40' y1='35' x2='40' y2='40' stroke='#2a3a4d' stroke-width='1.2'/>
                    <line x1='140' y1='35' x2='140' y2='40' stroke='#2a3a4d' stroke-width='1.2'/>
                    <line x1='140' y1='80' x2='140' y2='70' stroke='#2a3a4d' stroke-width='1.2'/>
                    <defs>
                      <marker id='arrow' markerWidth='8' markerHeight='8' refX='7' refY='4' orient='auto' markerUnits='strokeWidth'>
                        <path d='M0,0 L8,4 L0,8 L2,4 z' fill='#2a3a4d'/>
                      </marker>
                    </defs>
                  </svg>
                </div>
                <span style='font-size:0.97em;color:#3a4a5d;'>The modulator oscillator (left) modulates the frequency of the carrier oscillator (center). Each has its own envelope. The carrier output is the drum sound. Pitch envelope (Freq Env) shapes the attack.</span>
              </div>`
        },
        's1_group': {
            title: 'Rompler 1',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 1</b> plays back user samples with flexible slicing, pitch, and filter controls.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the output.</li>
                  <li><b>Gate</b>: Triggers sample playback.</li>
                  <li><b>Level</b>: Output volume (squared response).</li>
                  <li><b>Pan</b>: Stereo position.</li>
                  <li><b>FX1 Send</b>: Amount sent to delay effect (squared response).</li>
                  <li><b>FX2 Send</b>: Amount sent to reverb effect (squared response).</li>
                  <li><b>Speed</b>: Playback speed (0–2x, CV-controllable).</li>
                  <li><b>Pitch</b>: Pitch offset (CV-controllable).</li>
                  <li><b>Bank/Slice</b>: Selects sample bank and slice.</li>
                  <li><b>Start/End</b>: Start and end position within the slice (relative).</li>
                  <li><b>Loop/Loop PiPo</b>: Enables looping and ping-pong looping.</li>
                  <li><b>Loop Marker</b>: Position of loop marker (relative).</li>
                  <li><b>Attack/Decay</b>: Envelope attack and decay times.</li>
                  <li><b>EG→FM</b>: Envelope to FM amount.</li>
                  <li><b>Bit Reduction</b>: Reduces bit depth for lo-fi effects.</li>
                  <li><b>Filter Type</b>: Selects filter type (LPF, HPF, BPF, etc).</li>
                  <li><b>Cutoff/Resonance</b>: Filter cutoff and resonance.</li>
                </ul>
                <b>General:</b> Use Gate to trigger, and adjust slice/bank for sample selection. All parameters can be modulated for creative effects.
              </div>`
        },
        's2_group': {
            title: 'Rompler 2',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 2</b> is identical to Rompler 1, with independent parameters and sample selection. See Rompler 1 for parameter details.
              </div>`
        },
        's3_group': {
            title: 'Rompler 3',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 3</b> is identical to Rompler 1, with independent parameters and sample selection. See Rompler 1 for parameter details.
              </div>`
        },
        's4_group': {
            title: 'Rompler 4',
            body: `<div style="margin-bottom:1em;">
                <b>Rompler 4</b> is identical to Rompler 1, with independent parameters and sample selection. See Rompler 1 for parameter details.
              </div>`
        },
        'fx1_group': {
            title: 'Delay FX',
            body: `<div style="margin-bottom:1em;">
                <b>Delay FX</b> is a flexible stereo delay with feedback, filtering, and sync options.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Time (ms)</b>: Delay time in milliseconds (CV-controllable).</li>
                  <li><b>Sync</b>: Syncs delay time to external trigger.</li>
                  <li><b>Freeze</b>: Freezes the delay buffer.</li>
                  <li><b>Tape/Digital</b>: Switches between tape-style and digital delay.</li>
                  <li><b>Stereo Width</b>: Controls stereo spread of delay.</li>
                  <li><b>Reverb Send</b>: Amount sent from delay to reverb.</li>
                  <li><b>Feedback</b>: Amount of feedback (CV-controllable).</li>
                  <li><b>Base/Width</b>: Highpass and lowpass filter cutoff frequencies.</li>
                  <li><b>Amount</b>: Output level of the delay effect.</li>
                </ul>
                <b>General:</b> Use feedback and filtering for creative delay effects. Sync and freeze allow for rhythmic and experimental textures.
              </div>`
        },
        'fx2_group': {
            title: 'Reverb FX',
            body: `<div style="margin-bottom:1em;">
                <b>Reverb FX</b> is a stereo reverb with time, amount, and lowpass filter controls.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Time</b>: Reverb decay time (CV-controllable).</li>
                  <li><b>Amount</b>: Output level of the reverb effect (CV-controllable).</li>
                  <li><b>Lowpass</b>: Lowpass filter cutoff for the reverb tail (CV-controllable).</li>
                </ul>
                <b>General:</b> Use reverb to add space and depth to the drum mix.
              </div>`
        },
        'c_group': {
            title: 'Compressor',
            body: `<div style="margin-bottom:1em;">
                <b>Compressor</b> is a stereo bus compressor for the drum mix.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Threshold (dB)</b>: Level above which compression is applied (-80 to 0 dB).</li>
                  <li><b>Attack</b>: Attack time in ms (0.3–30 ms).</li>
                  <li><b>Release</b>: Release time in ms (40–2000 ms).</li>
                  <li><b>Ratio</b>: Compression ratio (0.0001–1.25).</li>
                  <li><b>Sidechain LPF</b>: Enables lowpass filtering of the sidechain signal.</li>
                  <li><b>Mix</b>: Dry/wet mix for parallel compression.</li>
                  <li><b>Gain</b>: Makeup gain in dB (0–60 dB).</li>
                  <li><b>Delay/Rev Level</b>: Amount of delay/reverb sent to compressor sidechain.</li>
                </ul>
                <b>General:</b> Use compressor to control dynamics and glue the drum mix together.
              </div>`
        },
        'sum': {
            title: 'Overall Mix',
            body: `<div style="margin-bottom:1em;">
                <b>Overall Mix</b> controls the final output level and mute for the drum rack.<br><br>
                <b>Parameters:</b>
                <ul style='margin:0 0 1em 1.2em;padding:0;font-size:1em;'>
                  <li><b>Mute</b>: Silences the entire drum rack output.</li>
                  <li><b>Level</b>: Final output level (squared response).</li>
                </ul>
                <b>General:</b> Use mute and level to control the overall output of the drum rack.
              </div>`
        }
    };

    // Show modal on help icon click
    $('#drumrack-ui').on('click', '.drumrack-group-help', function(e) {
        e.stopPropagation();
        const groupId = $(this).data('group-id');
        const help = modelHelpContent[groupId];
        if (help) {
            $('#drumrack-model-help-modal-content').html(`
                <h3 style="margin:0 0 1em 0;font-size:1.18em;font-weight:600;color:#2a3a4d;text-align:center;">${help.title} Help</h3>
                <div style="margin-bottom:1.2em;">${help.body}</div>
                <button id="drumrack-model-help-modal-close" style="margin:0.3em auto 0 auto;padding:0.55em 1.3em;border-radius:7px;border:none;background:#bfc7d1;color:#222;font-size:1em;font-weight:500;cursor:pointer;transition:background 0.2s;box-shadow:0 1px 4px rgba(0,0,0,0.04);min-width:110px;line-height:1.4;">Close</button>
            `);
            $('#drumrack-model-help-modal').fadeIn(120);
        }
    });
    // Close modal on button or outside click
    $(document).off('click.drumrack-model-help-modal').on('click.drumrack-model-help-modal', function(e) {
      if ($(e.target).is('#drumrack-model-help-modal, #drumrack-model-help-modal-close')) {
        $('#drumrack-model-help-modal').fadeOut(120);
      }
    });
});
