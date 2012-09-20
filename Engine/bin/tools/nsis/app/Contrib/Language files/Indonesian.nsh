;Language: Indonesian (1057)
;By Ariel825010106@yahoo.com [visit www.ariel106.cjb.net]

!insertmacro LANGFILE "Indonesian" "Indonesian"

!ifdef MUI_WELCOMEPAGE
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TITLE "Selamat datang di $(^NameDA) Setup Wizard"
  ${LangFileString} MUI_TEXT_WELCOME_INFO_TEXT "Setup Wizard akan membantu anda pada proses instalasi $(^NameDA).$\r$\n$\r$\nSangat disarankan untuk menutup program lainnya sebelum memulai Setup ini. Ini memungkinkan untuk merubah file yang dipakai oleh sistem tanpa harus me-reboot komputer anda.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_UNWELCOMEPAGE
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TITLE "Selamat datang di $(^NameDA) Uninstall Wizard"
  ${LangFileString} MUI_UNTEXT_WELCOME_INFO_TEXT "Uninstall Wizard akan membantu anda pada proses uninstalasi $(^NameDA).$\r$\n$\r$\nSebelum memulai uninstalasi, pastikan dulu $(^NameDA) tidak sedang berjalan.$\r$\n$\r$\n$_CLICK"
!endif

!ifdef MUI_LICENSEPAGE
  ${LangFileString} MUI_TEXT_LICENSE_TITLE "Perihal Lisensi"
  ${LangFileString} MUI_TEXT_LICENSE_SUBTITLE "Silahkan membaca lisensi berikut sebelum menginstall $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM "Jika anda menerima semua yang ada di lisensi, klik Saya setuju untuk melanjutkan. Anda harus setuju untuk dapat menginstall $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX "Jika anda menerima semua yang ada di lisensi, beri tanda centang. Anda harus setuju untuk dapat menginstall $(^NameDA). $_CLICK"
  ${LangFileString} MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Jika anda menerima semua yang ada di lisensi, pilihlah salah satu item dibawah ini. Anda harus setuju untuk dapat menginstall $(^NameDA). $_CLICK"
!endif

!ifdef MUI_UNLICENSEPAGE
  ${LangFileString} MUI_UNTEXT_LICENSE_TITLE "Perihal Lisensi"
  ${LangFileString} MUI_UNTEXT_LICENSE_SUBTITLE "Silahkan membaca lisensi berikut sebelum meng-uninstall $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM "Jika anda menerima semua yang ada di lisensi, klik Saya setuju untuk melanjutkan. Anda harus setuju untuk dapat meng-uninstall $(^NameDA)."
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX "Jika anda menerima semua yang ada di lisensi, beri tanda centang. Anda harus setuju untuk dapat meng-uninstall $(^NameDA). $_CLICK"
  ${LangFileString} MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS "Jika anda menerima semua yang ada di lisensi, pilihlah salah satu item dibawah ini. Anda harus setuju untuk dapat meng-uninstall $(^NameDA). $_CLICK"
!endif

!ifdef MUI_LICENSEPAGE | MUI_UNLICENSEPAGE
  ${LangFileString} MUI_INNERTEXT_LICENSE_TOP "Tekan Page Down untuk melihat selanjutnya."
!endif

!ifdef MUI_COMPONENTSPAGE
  ${LangFileString} MUI_TEXT_COMPONENTS_TITLE "Pilih Komponen"
  ${LangFileString} MUI_TEXT_COMPONENTS_SUBTITLE "Pilih fasilitas dari $(^NameDA) yang ingin di-install."
  ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE "Deskripsi"
!endif

!ifdef MUI_UNCOMPONENTSPAGE
  ${LangFileString} MUI_UNTEXT_COMPONENTS_TITLE "Pilih Komponen"
  ${LangFileString} MUI_UNTEXT_COMPONENTS_SUBTITLE "Pilih fasilitas dari $(^NameDA) yang ingin di-uninstall."
!endif

!ifdef MUI_COMPONENTSPAGE | MUI_UNCOMPONENTSPAGE
  !ifndef NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Tunjuk ke salah satu komponen untuk melihat deskripsi tentang komponen itu."
  !else
    ${LangFileString} MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO "Tunjuk ke salah satu komponen untuk melihat deskripsi tentang komponen itu."
  !endif
!endif

!ifdef MUI_DIRECTORYPAGE
  ${LangFileString} MUI_TEXT_DIRECTORY_TITLE "Pilih Lokasi Install"
  ${LangFileString} MUI_TEXT_DIRECTORY_SUBTITLE "Pilih folder untuk meng-install $(^NameDA)."
!endif

!ifdef MUI_UNDIRECTORYSPAGE
  ${LangFileString} MUI_UNTEXT_DIRECTORY_TITLE "Pilih Lokasi Uninstall"
  ${LangFileString} MUI_UNTEXT_DIRECTORY_SUBTITLE "Pilih folder untuk meng-uninstall $(^NameDA)."
!endif

!ifdef MUI_INSTFILESPAGE
  ${LangFileString} MUI_TEXT_INSTALLING_TITLE "Install"
  ${LangFileString} MUI_TEXT_INSTALLING_SUBTITLE "Mohon tunggu selama $(^NameDA) sedang di-install."
  ${LangFileString} MUI_TEXT_FINISH_TITLE "Instalasi Selesai"
  ${LangFileString} MUI_TEXT_FINISH_SUBTITLE "Setup sudah selesai."
  ${LangFileString} MUI_TEXT_ABORT_TITLE "Instalasi Dibatalkan"
  ${LangFileString} MUI_TEXT_ABORT_SUBTITLE "Setup belum selesai secara sempurna."
!endif

!ifdef MUI_UNINSTFILESPAGE
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_TITLE "Uninstall"
  ${LangFileString} MUI_UNTEXT_UNINSTALLING_SUBTITLE "Mohon tunggu selama $(^NameDA) sedang di-uninstall."
  ${LangFileString} MUI_UNTEXT_FINISH_TITLE "Proses Uninstall Selesai"
  ${LangFileString} MUI_UNTEXT_FINISH_SUBTITLE "Uninstall sudah selesai."
  ${LangFileString} MUI_UNTEXT_ABORT_TITLE "Proses Uninstall Dibatalkan"
  ${LangFileString} MUI_UNTEXT_ABORT_SUBTITLE "Uninstall belum selesai secara sempurna."
!endif

!ifdef MUI_FINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_INFO_TITLE "Menyelesaikan $(^NameDA) Setup Wizard"
  ${LangFileString} MUI_TEXT_FINISH_INFO_TEXT "$(^NameDA) sudah ter-install di komputer anda.$\r$\n$\r$\nKlik Selesai untuk menutup Setup Wizard."
  ${LangFileString} MUI_TEXT_FINISH_INFO_REBOOT "Komputer anda harus di reboot untuk menyelesaikan proses instalasi $(^NameDA). Apakah anda mau reboot sekarang juga?"
!endif

!ifdef MUI_UNFINISHPAGE
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TITLE "Menyelesaikan $(^NameDA) Uninstall Wizard"
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_TEXT "$(^NameDA) sudah ter-uninstall di komputer anda.$\r$\n$\r$\nKlik Selesai untuk menutup Setup Wizard."
  ${LangFileString} MUI_UNTEXT_FINISH_INFO_REBOOT "Komputer anda harus di reboot untuk menyelesaikan proses uninstall $(^NameDA). Apakah anda mau reboot sekarang juga?"
!endif

!ifdef MUI_FINISHPAGE | MUI_UNFINISHPAGE
  ${LangFileString} MUI_TEXT_FINISH_REBOOTNOW "Reboot sekarang"
  ${LangFileString} MUI_TEXT_FINISH_REBOOTLATER "Reboot nanti saja"
  ${LangFileString} MUI_TEXT_FINISH_RUN "&Jalankan $(^NameDA)"
  ${LangFileString} MUI_TEXT_FINISH_SHOWREADME "&Buka file Readme"
  ${LangFileString} MUI_BUTTONTEXT_FINISH "&Selesai"  
!endif

!ifdef MUI_STARTMENUPAGE
  ${LangFileString} MUI_TEXT_STARTMENU_TITLE "Pilih Folder Start Menu"
  ${LangFileString} MUI_TEXT_STARTMENU_SUBTITLE "Pilih folder Start Menu untuk meletakan shortcut $(^NameDA)."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_TOP "Pilih folder Start Menu dimana ingin diletakan shortcut program ini. Bisa juga memasukan nama folder yang belum ada untuk membuatnya."
  ${LangFileString} MUI_INNERTEXT_STARTMENU_CHECKBOX "Jangan buat shortcut"
!endif

!ifdef MUI_UNCONFIRMPAGE
  ${LangFileString} MUI_UNTEXT_CONFIRM_TITLE "Uninstall $(^NameDA)"
  ${LangFileString} MUI_UNTEXT_CONFIRM_SUBTITLE "Menghapus $(^NameDA) dari komputer anda."
!endif

!ifdef MUI_ABORTWARNING
  ${LangFileString} MUI_TEXT_ABORTWARNING "Apa anda yakin ingin menghentikan Setup $(^Name)?"
!endif

!ifdef MUI_UNABORTWARNING
  ${LangFileString} MUI_UNTEXT_ABORTWARNING "Apa anda yakin ingin menghentikan Uninstall $(^Name)?"
!endif
