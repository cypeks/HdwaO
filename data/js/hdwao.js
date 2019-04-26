function swiatlo_menu(){
  console.log('swiatlo');
  czysc_interwal();
  swiatlo_json();
  odswiez = setInterval(swiatlo_json, 6000);
}

function ustawienia_menu(){
  console.log('ustawienia');
  czysc_interwal();
  ustawienia_json();
  odswiez = setInterval(ustawienia_json, 15000);
}

function informacje_menu(){
  console.log('informacje');
  czysc_interwal();
  informacje_json();
}

function czysc_interwal(){
  if(typeof odswiez !== 'undefined') clearInterval(odswiez);
}

function select_option(a,b,krok){
  var html2 = '';
  for(j = a; j <= b; j = j+krok){
    html2 += '<option value="'+j+'">'+j+'</option>';
  }
  return html2;
}

function swiatlo_json(){
  console.log('swiatlo_json');
  $.getJSON( '/root.json', function(data) {
    var swiatlo = data.swiatlo;
    for (i = 1; i <= ilepwm; i++) {
      if(parseInt(swiatlo[i-1].wl)) $('#sw'+i).prop('checked',true);
      $('#pwm'+i).prop('value',swiatlo[i-1].pwm);
      $('#label-pwm'+i).text($('#pwm'+i).prop('value'));
      $('#wlacz'+i).prop('value',swiatlo[i-1].onh);
      $('#wylacz'+i).prop('value',swiatlo[i-1].offh);
      $('#czas'+i).prop('value',swiatlo[i-1].czas);
      $('#maxpwm'+i).prop('value',swiatlo[i-1].max);
      $('#odwrpwm'+i).prop('checked',parseInt(swiatlo[i-1].odwrpwm));
      if($('#sw'+i).prop('checked')) $('#divpwm'+i).collapse('show');
    }
  });
}

function ustawienia_json(){
  console.log('ustawienia_json');
  $.getJSON( '/ustawienia.json', function(data) {
    var data_godzina = data.data+' '+data.godzina;
      $('#data_godzina').prop('value',data_godzina);
  });
}

function informacje_json(){
  console.log('informacje_json');
  $.getJSON( '/info.json', function(data) {
    $('#ver').text(data.ver);
    $('#ilepwm').text(data.ilepwm);
    $('#hostname').text(data.hostname);
    $('#mac').text(data.mac);
    $('#free_flash').text(data.free_flash+' B');
    $('#free_ram').text(data.free_ram+' B');
    $('#vcc').text(data.vcc+' mV');
    $('#rtctemp').html(data.rtctemp+' &#8451;');
  });
}

function konfigpwm_json(id,typ,val){
  $.ajax({
      type: 'POST',
      url:'/konfigpwm.json',
      dataType : 'json',
      data: { id: id, typ: typ, val: val },
      success: function(result) {
        $('#pwm'+id).prop('value', result.pwm);
        $('#label-pwm'+id).text(result.pwm);
        //console.log(result.value);
      },
      error: function(jqXHR, errorText, errorThrown) { console.log('konfigpwm: '+errorText);	}
    });
}

function konstruktor(){
  //---swiatlo---
  var html='';
  for (var i = 1; i <= ilepwm; i++) {
    console.log('ilepwm inc: '+i);
    html += '<div class="custom-control custom-switch ml-1">\
          <input type="checkbox" class="custom-control-input swpwm" role="button" data-toggle="collapse" data-target="#divpwm'+i+'" aria-expanded="false" aria-controls="divpwm'+i+'" id="sw'+i+'" name="divpwm'+i+'">\
          <label class="custom-control-label" for="sw'+i+'"></label>\
        </div>\
        <div class="collapse" id="divpwm'+i+'">\
          <div class="card card-body">\
            <div class="row">\
              <div class="col-1 mt-3">\
                <label for="pwm'+i+'" id="label-pwm'+i+'"></label>\
              </div>\
              <div class="col mt-3">\
                <input type="range" class="custom-range pwm" min="0" max="100" id="pwm'+i+'">\
              </div>\
            </div>\
            <div class="row">\
              <div class="col mt-2">\
                <div class="input-group">\
                  <div class="input-group-prepend">\
                    <span class="input-group-text"><i class="material-icons">brightness_5</i></span>\
                  </div>\
                  <input class="form-control wlacz" type="text" id="wlacz'+i+'">\
                </div>\
              </div>\
              <div class="col mt-2">\
                <div class="input-group">\
                  <div class="input-group-prepend">\
                    <span class="input-group-text"><i class="material-icons">brightness_4</i></span>\
                  </div>\
                  <input class="form-control wylacz" type="text" id="wylacz'+i+'">\
                </div>\
              </div>\
            </div>\
            <div class="row">\
              <div class="col mt-2 mb-3">\
                <div class="input-group">\
                  <div class="input-group-prepend">\
                    <span class="input-group-text"><i class="material-icons">timelapse</i></span>\
                  </div>\
                  <select class="custom-select mr-sm-2 czas" id="czas'+i+'">';
        html += select_option(5,250,5);
        html += '</select>\
                </div>\
              </div>\
              <div class="col mt-2 mb-3">\
                <div class="input-group">\
                  <div class="input-group-prepend">\
                    <span class="input-group-text"><i class="material-icons">flare</i></span>\
                  </div>\
                  <select class="custom-select mr-sm-2 maxpwm" id="maxpwm'+i+'">';
        html += select_option(1,100,1);
        html += '</select>\
                </div>\
              </div>\
            </div>\
            <div class="row">\
              <div class="col mt-2 mb-2 text-center">\
                  <div class="custom-control custom-switch">\
                    <input type="checkbox" class="custom-control-input odwrpwm" id="odwrpwm'+i+'">\
                    <label class="custom-control-label" for="odwrpwm'+i+'"><i class="material-icons">invert_colors</i></label>\
                  </div>\
              </div>\
            </div>\
          </div>\
        </div>';
  }
  $('#nav-swiatlo').html(html);
  //---ustawienia---
  html = '<div class="row">\
            <div class="col">\
              <div class="input-group">\
                <div class="input-group-prepend">\
                  <span class="input-group-text"><i class="material-icons">access_time</i></span>\
                </div>\
                <input class="form-control data_godzina" type="text" id="data_godzina">\
              </div>\
            </div>\
          </div>\
          <div class="row mt-3">\
            <div class="col">\
              <button type="button" class="btn btn-primary" data-toggle="modal" data-target="#resetWiFiModal"><i class="material-icons">wifi</i> Reset WiFi</button>\
              <!-- Modal resetWiFiOK start -->\
              <div class="modal fade" id="resetWiFiModal" tabindex="-1" role="dialog" aria-labelledby="resetWiFiModalLabel" aria-hidden="true">\
                <div class="modal-dialog modal-dialog-centered" role="document">\
                  <div class="modal-content">\
                    <div class="modal-header">\
                      <h5 class="modal-title" id="resetWiFiModalLabel">Reset WiFi</h5>\
                      <button type="button" class="close" data-dismiss="modal" aria-label="Close">\
                        <span aria-hidden="true">&times;</span>\
                      </button>\
                    </div>\
                    <div class="modal-body alert-danger">\
                      Czy na pewno zresetować dane sieci WiFi?\
                    </div>\
                    <div class="modal-footer">\
                      <button type="button" class="btn btn-secondary" data-dismiss="modal">Anuluj</button>\
                      <button type="button" class="btn btn-primary" id="resetWiFiOK">OK</button>\
                    </div>\
                  </div>\
                </div>\
              </div>\
              <!-- Modal resetWiFiOK stop -->\
              <!-- Modal infoWiFi start -->\
              <div class="modal fade" id="infoWiFiModal" tabindex="-1" role="dialog" aria-labelledby="infoWiFiModalLabel" aria-hidden="true">\
                <div class="modal-dialog modal-dialog-centered" role="document">\
                  <div class="modal-content">\
                    <div class="modal-header">\
                      <h5 class="modal-title" id="infoWiFiLabel">Reset WiFi</h5>\
                      <button type="button" class="close" data-dismiss="modal" aria-label="Close">\
                        <span aria-hidden="true">&times;</span>\
                      </button>\
                    </div>\
                    <div class="modal-body alert-success">\
                      Dane WiFi zostały zresetowane do ustawień fabrycznych.\
                    </div>\
                    <div class="modal-footer">&nbsp;</div>\
                  </div>\
                </div>\
              </div>\
              <!-- Modal infoWiFi stop -->\
            </div>\
          </div>';
  $('#nav-ustawienia').html(html);
  //---informacje---
  html = '<div class="row mt-3">\
            <div class="col">Hostname:</div><div class="col" id="hostname"></div>\
          </div>\
          <div class="row mt-3">\
            <div class="col">Wersja:</div><div class="col" id="ver"></div>\
          </div>\
          <div class="row mt-3">\
                    <div class="col">Kanały:</div><div class="col" id="ilepwm"></div>\
          </div>\
          <div class="row mt-3">\
                    <div class="col">MAC:</div><div class="col" id="mac"></div>\
          </div>\
          <div class="row mt-3">\
                    <div class="col">VCC:</div><div class="col" id="vcc"></div>\
          </div>\
          <div class="row mt-3">\
                    <div class="col">Dostępny flash:</div><div class="col" id="free_flash"></div>\
          </div>\
          <div class="row mt-3">\
                    <div class="col">Dostępny RAM:</div><div class="col" id="free_ram"></div>\
          </div>\
          <div class="row mt-3">\
                    <div class="col">Temp. RTC:</div><div class="col" id="rtctemp"></div>\
          </div>';
  $('#nav-informacje').html(html);

  swiatlo_menu();
  for (i = 1; i <= ilepwm; i++) {
    $('#wlacz'+i).bootstrapMaterialDatePicker({ format : 'HH:mm', lang : 'pl', switchOnClick: true, cancelText: 'Anuluj', date: false, nowButton: false });
    $('#wylacz'+i).bootstrapMaterialDatePicker({ format : 'HH:mm', lang : 'pl', switchOnClick: true, cancelText: 'Anuluj', date: false, nowButton: false });
  }
  $('#data_godzina').bootstrapMaterialDatePicker({ format : 'YYYY/MM/DD HH:mm', lang : 'pl', switchOnClick: true, cancelText: 'Anuluj', date: true, nowButton: true, nowText: 'Pobierz' });
}
